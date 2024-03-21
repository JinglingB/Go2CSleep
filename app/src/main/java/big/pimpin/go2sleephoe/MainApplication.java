package big.pimpin.go2sleephoe;

import android.app.Application;

import java.io.File;

import dalvik.annotation.optimization.FastNative;

public final class MainApplication extends Application {
    static {
        System.loadLibrary("go2sleephoe");
    }

    /** @noinspection SameParameterValue*/
    private static Class<?> keepMyClassForNamesR8(final String className) throws ClassNotFoundException {
        //noinspection ConcatenationWithEmptyString
        return Class.forName(className + "");
    }

    @Override
    public void onCreate() {
        super.onCreate();

        final File privateKeyFile = new File(getFilesDir(), "id_ed25519");
        final File publicKeyFile = new File(getExternalFilesDir(null), "id_ed25519.pub");

        try {
            if (!privateKeyFile.exists() || !publicKeyFile.exists())
                keepMyClassForNamesR8("big.pimpin.go2sleephoe.Ed25519Keygen").getMethod("generateAndWriteEd25519KeyPair", File.class, File.class).invoke(null, privateKeyFile, publicKeyFile);
        } catch (final Throwable th) {
            throw new RuntimeException(th);
        }

        set_key_paths(privateKeyFile.getAbsolutePath(), publicKeyFile.getAbsolutePath());
    }

    @FastNative
    private static native void set_key_paths(final String idEd25519Path, final String idEd25519PubPath);
}
