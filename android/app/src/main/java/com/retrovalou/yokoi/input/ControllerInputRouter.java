package com.retrovalou.yokoi.input;

import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.retrovalou.yokoi.nativebridge.YokoiNative;

public final class ControllerInputRouter {
    private int controllerMask;
    private int overlayMask;
    private long lastDpadKeyEventUptimeMs;

    public void clearAll() {
        controllerMask = 0;
        overlayMask = 0;
        pushControllerMask();
    }

    public int getCombinedMask() {
        return controllerMask | overlayMask;
    }

    public void setOverlayBit(int bit, boolean down) {
        int newMask = down ? (overlayMask | bit) : (overlayMask & ~bit);
        if (newMask != overlayMask) {
            overlayMask = newMask;
            pushControllerMask();
        }
    }

    private void setControllerBit(int bit, boolean down) {
        int newMask = down ? (controllerMask | bit) : (controllerMask & ~bit);
        if (newMask != controllerMask) {
            controllerMask = newMask;
            pushControllerMask();
        }
    }

    private void pushControllerMask() {
        YokoiNative.nativeSetControllerMask(getCombinedMask());
    }

    private static boolean isControllerEvent(KeyEvent event) {
        if (event == null) return false;
        int src = event.getSource();
        return ((src & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
                || ((src & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
                || ((src & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD);
    }

    private static boolean isJoystickEvent(MotionEvent event) {
        if (event == null) return false;
        int src = event.getSource();
        return ((src & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK);
    }

    private static float getCenteredAxis(MotionEvent event, InputDevice device, int axis) {
        if (device == null) return 0.0f;
        InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());
        if (range == null) return 0.0f;
        float value = event.getAxisValue(axis);
        float flat = range.getFlat();
        if (Math.abs(value) <= flat) return 0.0f;
        return value;
    }

    public boolean dispatchKeyEvent(KeyEvent event) {
        if (!isControllerEvent(event)) {
            return false;
        }

        int action = event.getAction();
        boolean down = action == KeyEvent.ACTION_DOWN;
        int keyCode = event.getKeyCode();

        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_UP:
                lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                setControllerBit(ControllerBits.CTL_DPAD_UP, down);
                return true;
            case KeyEvent.KEYCODE_DPAD_DOWN:
                lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                setControllerBit(ControllerBits.CTL_DPAD_DOWN, down);
                return true;
            case KeyEvent.KEYCODE_DPAD_LEFT:
                lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                setControllerBit(ControllerBits.CTL_DPAD_LEFT, down);
                return true;
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                lastDpadKeyEventUptimeMs = android.os.SystemClock.uptimeMillis();
                setControllerBit(ControllerBits.CTL_DPAD_RIGHT, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_A:
                setControllerBit(ControllerBits.CTL_A, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_B:
                setControllerBit(ControllerBits.CTL_B, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_X:
                setControllerBit(ControllerBits.CTL_X, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_Y:
                setControllerBit(ControllerBits.CTL_Y, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_START:
                setControllerBit(ControllerBits.CTL_START, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_SELECT:
                setControllerBit(ControllerBits.CTL_SELECT, down);
                return true;
            case KeyEvent.KEYCODE_BUTTON_L1:
            case KeyEvent.KEYCODE_BUTTON_L2:
                setControllerBit(ControllerBits.CTL_L1, down);
                return true;
            default:
                return false;
        }
    }

    public boolean onGenericMotionEvent(MotionEvent event) {
        if (!isJoystickEvent(event)) {
            return false;
        }

        long now = android.os.SystemClock.uptimeMillis();
        if (now - lastDpadKeyEventUptimeMs < 200) {
            return true;
        }

        final float dead = 0.35f;
        InputDevice dev = event.getDevice();

        float x = getCenteredAxis(event, dev, MotionEvent.AXIS_HAT_X);
        float y = getCenteredAxis(event, dev, MotionEvent.AXIS_HAT_Y);

        if (x == 0.0f && y == 0.0f) {
            x = getCenteredAxis(event, dev, MotionEvent.AXIS_X);
            y = getCenteredAxis(event, dev, MotionEvent.AXIS_Y);
        }

        boolean left = x < -dead;
        boolean right = x > dead;
        boolean up = y < -dead;
        boolean down = y > dead;

        int newMask = controllerMask;
        newMask = left ? (newMask | ControllerBits.CTL_DPAD_LEFT) : (newMask & ~ControllerBits.CTL_DPAD_LEFT);
        newMask = right ? (newMask | ControllerBits.CTL_DPAD_RIGHT) : (newMask & ~ControllerBits.CTL_DPAD_RIGHT);
        newMask = up ? (newMask | ControllerBits.CTL_DPAD_UP) : (newMask & ~ControllerBits.CTL_DPAD_UP);
        newMask = down ? (newMask | ControllerBits.CTL_DPAD_DOWN) : (newMask & ~ControllerBits.CTL_DPAD_DOWN);

        if (newMask != controllerMask) {
            controllerMask = newMask;
            pushControllerMask();
        }
        return true;
    }
}
