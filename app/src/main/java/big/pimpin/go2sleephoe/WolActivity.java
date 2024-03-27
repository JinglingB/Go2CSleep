package big.pimpin.go2sleephoe;

import android.app.Activity;
import android.os.Bundle;

import dalvik.annotation.optimization.CriticalNative;

public final class WolActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        wol();
        finishAndRemoveTask();
    }

    @CriticalNative
    private static native void wol();
}
