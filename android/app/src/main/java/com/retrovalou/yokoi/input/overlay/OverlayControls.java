package com.retrovalou.yokoi.input.overlay;

import android.app.Activity;
import android.content.res.Configuration;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.retrovalou.yokoi.input.ControllerBits;
import com.retrovalou.yokoi.input.ControllerInputRouter;
import com.retrovalou.yokoi.prefs.AppPrefs;

public final class OverlayControls {
    private final Activity activity;
    private final ControllerInputRouter router;

    private FrameLayout overlayRoot;
    private boolean enabled;

    public OverlayControls(Activity activity, ControllerInputRouter router) {
        this.activity = activity;
        this.router = router;
        this.enabled = AppPrefs.getShowOverlayControls(activity);
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
        AppPrefs.setShowOverlayControls(activity, enabled);
        activity.runOnUiThread(() -> {
            ensureCreated();
            overlayRoot.setVisibility(enabled ? View.VISIBLE : View.GONE);
        });

        if (!enabled) {
            // Clear overlay bits when hidden.
            router.setOverlayBit(ControllerBits.CTL_DPAD_UP, false);
            router.setOverlayBit(ControllerBits.CTL_DPAD_DOWN, false);
            router.setOverlayBit(ControllerBits.CTL_DPAD_LEFT, false);
            router.setOverlayBit(ControllerBits.CTL_DPAD_RIGHT, false);
            router.setOverlayBit(ControllerBits.CTL_A, false);
            router.setOverlayBit(ControllerBits.CTL_B, false);
            router.setOverlayBit(ControllerBits.CTL_X, false);
            router.setOverlayBit(ControllerBits.CTL_Y, false);
            router.setOverlayBit(ControllerBits.CTL_START, false);
            router.setOverlayBit(ControllerBits.CTL_SELECT, false);
            router.setOverlayBit(ControllerBits.CTL_L1, false);
        }
    }

    public View getView() {
        ensureCreated();
        overlayRoot.setVisibility(enabled ? View.VISIBLE : View.GONE);
        return overlayRoot;
    }

    private int dp(int v) {
        return (int) (v * activity.getResources().getDisplayMetrics().density + 0.5f);
    }

    private int dpF(float v) {
        return (int) (v * activity.getResources().getDisplayMetrics().density + 0.5f);
    }

    private GradientDrawable makeOverlayShape(boolean oval, float cornerRadiusPx) {
        GradientDrawable d = new GradientDrawable();
        d.setShape(oval ? GradientDrawable.OVAL : GradientDrawable.RECTANGLE);
        if (!oval) {
            d.setCornerRadius(cornerRadiusPx);
        }
        d.setColor(Color.argb(110, 20, 20, 20));
        d.setStroke(dp(1), Color.argb(120, 255, 255, 255));
        return d;
    }

    private Button makeOverlayButton(String label, int bit, int widthPx, int heightPx, boolean circle, boolean pill) {
        Button b = new Button(activity);
        b.setText(label);
        b.setAllCaps(false);
        b.setTextColor(Color.argb(220, 255, 255, 255));
        b.setTextSize(13);
        b.setPadding(0, 0, 0, 0);
        b.setMinWidth(0);
        b.setMinHeight(0);

        final boolean oval = circle;
        final float corner = pill ? (heightPx * 0.5f) : dp(10);
        b.setBackground(makeOverlayShape(oval, corner));
        b.setAlpha(0.65f);
        b.setOnTouchListener((v, event) -> {
            int a = event.getActionMasked();
            if (a == MotionEvent.ACTION_DOWN || a == MotionEvent.ACTION_POINTER_DOWN) {
                router.setOverlayBit(bit, true);
                v.setPressed(true);
                v.setAlpha(0.9f);
                return true;
            }
            if (a == MotionEvent.ACTION_UP || a == MotionEvent.ACTION_POINTER_UP || a == MotionEvent.ACTION_CANCEL) {
                router.setOverlayBit(bit, false);
                v.setPressed(false);
                v.setAlpha(0.65f);
                return true;
            }
            return true;
        });

        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(widthPx, heightPx);
        lp.leftMargin = dp(4);
        lp.rightMargin = dp(4);
        lp.topMargin = dp(4);
        lp.bottomMargin = dp(4);
        b.setLayoutParams(lp);
        return b;
    }

    private void ensureCreated() {
        if (overlayRoot != null) {
            return;
        }

        overlayRoot = new FrameLayout(activity);
        overlayRoot.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        overlayRoot.setClickable(false);
        overlayRoot.setFocusable(false);

        final float density = activity.getResources().getDisplayMetrics().density;
        final float widthDp = activity.getResources().getDisplayMetrics().widthPixels / density;
        final float heightDp = activity.getResources().getDisplayMetrics().heightPixels / density;

        final float baseBtnDp = 58.0f;
        final float baseDpadDp = 170.0f;
        final float marginDp = 16.0f;

        // ABXY max row width is ~ (3 * btn + 16dp) due to per-button margins.
        final float requiredWidthDp = baseDpadDp + (3.0f * baseBtnDp) + (2.0f * marginDp) + 16.0f;
        final float scaleW = widthDp / requiredWidthDp;
        final float scaleH = heightDp / 640.0f;
        float scale = Math.min(1.0f, Math.min(scaleW, scaleH));
        scale = Math.max(0.75f, scale);

        final int btn = dpF(baseBtnDp * scale);
        final int margin = dpF(marginDp);
        final boolean landscape = activity.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE;

        // Start/Select sizing: slim in all cases, keep original placement rules.
        final int ssW = dp(112);
        final int ssH = dp(36);

        final int portraitBottomClearance = margin + ssH + dp(12);

        // D-pad
        DpadView dpad = new DpadView(activity, router::setOverlayBit);
        int dpadSize = dpF(baseDpadDp * scale);

        FrameLayout.LayoutParams dpadLp = new FrameLayout.LayoutParams(dpadSize, dpadSize);
        dpadLp.gravity = Gravity.BOTTOM | Gravity.START;
        dpadLp.leftMargin = margin;
        dpadLp.bottomMargin = landscape ? margin : portraitBottomClearance;
        overlayRoot.addView(dpad, dpadLp);

        // ABXY
        LinearLayout abxy = new LinearLayout(activity);
        abxy.setOrientation(LinearLayout.VERTICAL);

        LinearLayout abxyRowUp = new LinearLayout(activity);
        abxyRowUp.setOrientation(LinearLayout.HORIZONTAL);
        abxyRowUp.setGravity(Gravity.CENTER_HORIZONTAL);
        View abxySpacerL = new View(activity);
        abxySpacerL.setLayoutParams(new LinearLayout.LayoutParams(btn, btn));
        abxyRowUp.addView(abxySpacerL);
        abxyRowUp.addView(makeOverlayButton("X", ControllerBits.CTL_X, btn, btn, true, false));
        View abxySpacerR = new View(activity);
        abxySpacerR.setLayoutParams(new LinearLayout.LayoutParams(btn, btn));
        abxyRowUp.addView(abxySpacerR);

        LinearLayout abxyRowMid = new LinearLayout(activity);
        abxyRowMid.setOrientation(LinearLayout.HORIZONTAL);
        abxyRowMid.setGravity(Gravity.CENTER_HORIZONTAL);
        abxyRowMid.addView(makeOverlayButton("Y", ControllerBits.CTL_Y, btn, btn, true, false));
        View abxySpacer = new View(activity);
        abxySpacer.setLayoutParams(new LinearLayout.LayoutParams(btn, btn));
        abxyRowMid.addView(abxySpacer);
        abxyRowMid.addView(makeOverlayButton("A", ControllerBits.CTL_A, btn, btn, true, false));

        LinearLayout abxyRowDown = new LinearLayout(activity);
        abxyRowDown.setOrientation(LinearLayout.HORIZONTAL);
        abxyRowDown.setGravity(Gravity.CENTER_HORIZONTAL);
        View abxySpacerDL = new View(activity);
        abxySpacerDL.setLayoutParams(new LinearLayout.LayoutParams(btn, btn));
        abxyRowDown.addView(abxySpacerDL);
        abxyRowDown.addView(makeOverlayButton("B", ControllerBits.CTL_B, btn, btn, true, false));
        View abxySpacerDR = new View(activity);
        abxySpacerDR.setLayoutParams(new LinearLayout.LayoutParams(btn, btn));
        abxyRowDown.addView(abxySpacerDR);

        abxy.addView(abxyRowUp);
        abxy.addView(abxyRowMid);
        abxy.addView(abxyRowDown);

        FrameLayout.LayoutParams abxyLp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        abxyLp.gravity = Gravity.BOTTOM | Gravity.END;
        abxyLp.rightMargin = margin;
        abxyLp.bottomMargin = landscape ? margin : portraitBottomClearance;
        overlayRoot.addView(abxy, abxyLp);

        // Start/Select
        Button select = makeOverlayButton("Select", ControllerBits.CTL_SELECT, ssW, ssH, false, true);
        Button start = makeOverlayButton("Start", ControllerBits.CTL_START, ssW, ssH, false, true);

        // Important: FrameLayout overrides the child's LayoutParams. Use explicit sizes here
        // so Start/Select actually match ssW/ssH (and therefore keep the intended pill shape).
        FrameLayout.LayoutParams selectLp = new FrameLayout.LayoutParams(ssW, ssH);
        FrameLayout.LayoutParams startLp = new FrameLayout.LayoutParams(ssW, ssH);

        if (landscape) {
            selectLp.gravity = Gravity.TOP | Gravity.START;
            selectLp.leftMargin = margin;
            selectLp.topMargin = margin;

            startLp.gravity = Gravity.TOP | Gravity.END;
            startLp.rightMargin = margin;
            startLp.topMargin = margin;
        } else {
            selectLp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
            selectLp.bottomMargin = margin;
            selectLp.rightMargin = dp(60);

            startLp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
            startLp.bottomMargin = margin;
            startLp.leftMargin = dp(60);
        }

        overlayRoot.addView(select, selectLp);
        overlayRoot.addView(start, startLp);
    }
}
