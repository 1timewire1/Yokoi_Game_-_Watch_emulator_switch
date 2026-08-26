package com.retrovalou.yokoi.input;

// Must match native ControllerBits in android/app/src/main/cpp/yokoi_jni.cpp
public final class ControllerBits {
    private ControllerBits() {
    }

    public static final int CTL_DPAD_UP = 1 << 0;
    public static final int CTL_DPAD_DOWN = 1 << 1;
    public static final int CTL_DPAD_LEFT = 1 << 2;
    public static final int CTL_DPAD_RIGHT = 1 << 3;
    public static final int CTL_A = 1 << 4;
    public static final int CTL_B = 1 << 5;
    public static final int CTL_X = 1 << 6;
    public static final int CTL_Y = 1 << 7;
    public static final int CTL_START = 1 << 8;
    public static final int CTL_SELECT = 1 << 9;
    public static final int CTL_L1 = 1 << 10;
}
