package com.retrovalou.yokoi.input.overlay;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.View;

import com.retrovalou.yokoi.input.ControllerBits;

final class DpadView extends View {
    interface OverlayBitSink {
        void setOverlayBit(int bit, boolean down);
    }

    private final Paint fillPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint strokePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Paint pressedPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final OverlayBitSink sink;

    private int activeBit = 0;

    DpadView(Context ctx, OverlayBitSink sink) {
        super(ctx);
        this.sink = sink;

        setAlpha(0.75f);
        setClickable(true);

        fillPaint.setStyle(Paint.Style.FILL);
        fillPaint.setColor(Color.argb(110, 20, 20, 20));

        pressedPaint.setStyle(Paint.Style.FILL);
        pressedPaint.setColor(Color.argb(160, 255, 255, 255));

        strokePaint.setStyle(Paint.Style.STROKE);
        strokePaint.setStrokeWidth(dp(1));
        strokePaint.setColor(Color.argb(120, 255, 255, 255));
    }

    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        float w = getWidth();
        float h = getHeight();
        float cx = w * 0.5f;
        float cy = h * 0.5f;

        float arm = Math.min(w, h) * 0.33f;
        float thick = Math.min(w, h) * 0.26f;
        float r = thick * 0.35f;

        RectF center = new RectF(cx - thick * 0.5f, cy - thick * 0.5f, cx + thick * 0.5f, cy + thick * 0.5f);
        RectF up = new RectF(cx - thick * 0.5f, cy - arm - thick * 0.5f, cx + thick * 0.5f, cy - thick * 0.5f);
        RectF down = new RectF(cx - thick * 0.5f, cy + thick * 0.5f, cx + thick * 0.5f, cy + arm + thick * 0.5f);
        RectF left = new RectF(cx - arm - thick * 0.5f, cy - thick * 0.5f, cx - thick * 0.5f, cy + thick * 0.5f);
        RectF right = new RectF(cx + thick * 0.5f, cy - thick * 0.5f, cx + arm + thick * 0.5f, cy + thick * 0.5f);

        RectF vert = new RectF(cx - thick * 0.5f, cy - arm - thick * 0.5f, cx + thick * 0.5f, cy + arm + thick * 0.5f);
        RectF hori = new RectF(cx - arm - thick * 0.5f, cy - thick * 0.5f, cx + arm + thick * 0.5f, cy + thick * 0.5f);

        canvas.drawRoundRect(vert, r, r, fillPaint);
        canvas.drawRoundRect(hori, r, r, fillPaint);
        canvas.drawRoundRect(center, r, r, fillPaint);

        if (activeBit == ControllerBits.CTL_DPAD_UP) canvas.drawRoundRect(up, r, r, pressedPaint);
        if (activeBit == ControllerBits.CTL_DPAD_DOWN) canvas.drawRoundRect(down, r, r, pressedPaint);
        if (activeBit == ControllerBits.CTL_DPAD_LEFT) canvas.drawRoundRect(left, r, r, pressedPaint);
        if (activeBit == ControllerBits.CTL_DPAD_RIGHT) canvas.drawRoundRect(right, r, r, pressedPaint);

        Path pv = new Path();
        Path ph = new Path();
        pv.addRoundRect(vert, r, r, Path.Direction.CW);
        ph.addRoundRect(hori, r, r, Path.Direction.CW);
        pv.op(ph, Path.Op.UNION);
        canvas.drawPath(pv, strokePaint);
    }

    private int pickBit(float x, float y) {
        float w = getWidth();
        float h = getHeight();
        float cx = w * 0.5f;
        float cy = h * 0.5f;
        float dx = x - cx;
        float dy = y - cy;
        float dead = Math.min(w, h) * 0.12f;
        if (Math.abs(dx) < dead && Math.abs(dy) < dead) {
            return 0;
        }
        if (Math.abs(dx) > Math.abs(dy)) {
            return (dx < 0) ? ControllerBits.CTL_DPAD_LEFT : ControllerBits.CTL_DPAD_RIGHT;
        }
        return (dy < 0) ? ControllerBits.CTL_DPAD_UP : ControllerBits.CTL_DPAD_DOWN;
    }

    private void setActiveBit(int bit) {
        if (bit == activeBit) {
            return;
        }
        if (activeBit != 0) {
            sink.setOverlayBit(activeBit, false);
        }
        activeBit = bit;
        if (activeBit != 0) {
            sink.setOverlayBit(activeBit, true);
        }
        invalidate();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int a = event.getActionMasked();
        if (a == MotionEvent.ACTION_DOWN || a == MotionEvent.ACTION_MOVE) {
            setActiveBit(pickBit(event.getX(), event.getY()));
            return true;
        }
        if (a == MotionEvent.ACTION_UP || a == MotionEvent.ACTION_CANCEL) {
            setActiveBit(0);
            return true;
        }
        return super.onTouchEvent(event);
    }
}
