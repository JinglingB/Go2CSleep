package big.pimpin.go2sleephoe;

import android.app.Activity;
import android.os.Bundle;

import dalvik.annotation.optimization.FastNative;

abstract class BaseSshActivity extends Activity {
    private final String sshCommand;

    BaseSshActivity(final String sshCommand) {
        super();
        this.sshCommand = sshCommand;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ssh2_exec(sshCommand);
        //android.widget.Toast.makeText(this, "Wir suchen dich", android.widget.Toast.LENGTH_LONG).show();
        finishAndRemoveTask();
    }

    @FastNative
    private static native void ssh2_exec(final String commandLine);
}
