package com.mojang.minecraftpe;

import com.mojang.android.StringValue;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.ToggleButton;

public class BiomeGrassTintButton extends ToggleButton implements OnClickListener, StringValue {
    public BiomeGrassTintButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        _init();
    }

    public void onClick(View v) {
        _update();
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        _update();
    }

    @Override
    protected void onAttachedToWindow() {
        if (!_attached) {
            _update();
            _attached = true;
        }
    }

    private boolean _attached = false;

    private void _init() {
        setOnClickListener(this);
        setChecked(true);
    }

    private void _update() {
        View v = getRootView().findViewById(R.id.labelBiomeGrassTintDesc);
        if (v != null && v instanceof TextView) {
            ((TextView) v).setText(isChecked()
                ? "Grass block top color changes by biome: ON"
                : "Grass block top color changes by biome: OFF");
        }
    }

    public String getStringValue() {
        return isChecked() ? "true" : "false";
    }
}
