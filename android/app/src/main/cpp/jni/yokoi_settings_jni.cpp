#include <jni.h>

#include <cstdint>

#include "std/settings.h"

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetBackgroundColor(JNIEnv*, jclass) {
    return (jint)g_settings.background_color;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetBackgroundColor(JNIEnv*, jclass, jint rgb) {
    // Settings use 0xRRGGBB.
    g_settings.background_color = (uint32_t)rgb & 0x00FFFFFFu;
    save_settings();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetSegmentMarkingAlpha(JNIEnv*, jclass) {
    return (jint)g_settings.segment_marking_alpha;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetSegmentMarkingAlpha(JNIEnv*, jclass, jint alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    g_settings.segment_marking_alpha = (uint8_t)alpha;
    save_settings();
}
