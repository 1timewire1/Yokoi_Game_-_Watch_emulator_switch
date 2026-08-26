package com.retrovalou.yokoi.audio;

import android.media.AudioAttributes;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import com.retrovalou.yokoi.nativebridge.YokoiNative;

public final class AudioDriver {
    private AudioTrack audioTrack;
    private Thread audioThread;
    private volatile boolean audioRunning;

    public void stop() {
        try {
            YokoiNative.nativeStopAaudio();
        } catch (Throwable ignored) {
        }

        audioRunning = false;
        if (audioThread != null) {
            try {
                audioThread.join(500);
            } catch (InterruptedException ignored) {
            }
            audioThread = null;
        }
        if (audioTrack != null) {
            try {
                audioTrack.pause();
                audioTrack.flush();
                audioTrack.stop();
            } catch (IllegalStateException ignored) {
            }
            audioTrack.release();
            audioTrack = null;
        }
    }

    public void startIfNeeded() {
        try {
            YokoiNative.nativeStartAaudio();
            return;
        } catch (Throwable ignored) {
        }

        if (audioTrack != null || audioThread != null) {
            return;
        }

        final int sourceRate = Math.max(1, YokoiNative.nativeGetAudioSampleRate());
        final int framesPerWrite = 256;

        int sampleRate = sourceRate;
        int minBytes = AudioTrack.getMinBufferSize(
                sampleRate,
                AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        if (minBytes <= 0) {
            sampleRate = 48000;
            minBytes = AudioTrack.getMinBufferSize(
                    sampleRate,
                    AudioFormat.CHANNEL_OUT_MONO,
                    AudioFormat.ENCODING_PCM_16BIT);
            if (minBytes <= 0) {
                sampleRate = 44100;
                minBytes = AudioTrack.getMinBufferSize(
                        sampleRate,
                        AudioFormat.CHANNEL_OUT_MONO,
                        AudioFormat.ENCODING_PCM_16BIT);
            }
            if (minBytes <= 0) {
                return;
            }
        }

        int bytesPerFrame = 2;
        int minForWrites = framesPerWrite * bytesPerFrame * 4;
        int bufferBytes = Math.max(minBytes * 2, minForWrites);

        audioTrack = new AudioTrack.Builder()
                .setAudioAttributes(new AudioAttributes.Builder()
                        .setLegacyStreamType(AudioManager.STREAM_MUSIC)
                        .setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                        .build())
                .setAudioFormat(new AudioFormat.Builder()
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                        .setSampleRate(sampleRate)
                        .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                        .build())
                .setTransferMode(AudioTrack.MODE_STREAM)
                .setBufferSizeInBytes(bufferBytes)
                .setPerformanceMode(AudioTrack.PERFORMANCE_MODE_LOW_LATENCY)
                .build();

        if (audioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            audioTrack.release();
            audioTrack = null;
            return;
        }

        audioTrack.play();
        audioRunning = true;

        final int outputRate = sampleRate;
        audioThread = new Thread(() -> {
            short[] pcm = new short[framesPerWrite];

            final boolean needResample = outputRate != sourceRate;
            final float step = needResample ? (sourceRate / (float) outputRate) : 1.0f;
            float srcPos = 0.0f;
            final short[] src = needResample ? new short[512] : null;
            int srcCount = 0;
            int srcIndex = 0;

            if (needResample) {
                srcCount = YokoiNative.nativeAudioRead(src, src.length);
                srcIndex = 0;
                srcPos = 0.0f;
            }

            while (audioRunning) {
                if (!needResample) {
                    YokoiNative.nativeAudioRead(pcm, framesPerWrite);
                } else {
                    for (int i = 0; i < framesPerWrite; i++) {
                        int i0 = srcIndex + (int) srcPos;
                        float frac = srcPos - (int) srcPos;
                        while (i0 + 1 >= srcCount) {
                            int remain = Math.max(0, srcCount - srcIndex);
                            if (remain > 0) {
                                System.arraycopy(src, srcIndex, src, 0, remain);
                            }
                            int got = YokoiNative.nativeAudioRead(src, src.length - remain);
                            srcCount = remain + Math.max(0, got);
                            srcIndex = 0;
                            srcPos = 0.0f;
                            i0 = 0;
                            frac = 0.0f;
                            if (srcCount <= 1) {
                                break;
                            }
                        }
                        short s0 = (srcCount > 0) ? src[i0] : 0;
                        short s1 = (srcCount > 1) ? src[i0 + 1] : s0;
                        pcm[i] = (short) (s0 + (s1 - s0) * frac);
                        srcPos += step;
                        int adv = (int) srcPos;
                        if (adv > 0) {
                            srcIndex += adv;
                            srcPos -= adv;
                        }
                    }
                }

                int wrote = audioTrack.write(pcm, 0, framesPerWrite);
                if (wrote <= 0) {
                    try {
                        Thread.sleep(5);
                    } catch (InterruptedException ignored) {
                    }
                }
            }
        }, "yokoi-audio");

        audioThread.setDaemon(true);
        audioThread.start();
    }
}
