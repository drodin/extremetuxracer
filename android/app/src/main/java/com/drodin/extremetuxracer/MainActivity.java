package com.drodin.extremetuxracer;

import android.app.NativeActivity;
import android.graphics.Point;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.View;

public class MainActivity extends NativeActivity {
    protected int width, height;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        getNativeResolution();

        setImmersiveSticky();

        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener
                (visibility -> setImmersiveSticky());

        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        setImmersiveSticky();

        super.onResume();
    }

    protected void getNativeResolution() {
        Display display = getWindowManager().getDefaultDisplay();
        Point size = new Point();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            display.getRealSize(size);
        } else {
            display.getSize(size);
        }
        width = size.x;
        height = size.y;
    }

    private void setImmersiveSticky() {
        int flags = View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            flags |= View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        }

        getWindow().getDecorView().setSystemUiVisibility(flags);
    }
}