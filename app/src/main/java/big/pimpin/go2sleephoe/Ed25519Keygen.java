package big.pimpin.go2sleephoe;

import android.os.Build;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Reader;
import java.io.Writer;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.security.SecureRandom;
import java.util.Base64;

/* A non-Reflection version of this can be found here:
   https://github.com/JinglingB/Go2Sleep/blob/main/app/src/main/java/big/pimpin/go2sleephoe/MainApplication.java */

final class Ed25519Keygen {
    private static Class<?> keepMyClassForNamesR8(final String className) throws ClassNotFoundException {
        //noinspection StringOperationCanBeSimplified
        return Class.forName(className.substring(0));
    }

    static void generateAndWriteEd25519KeyPair(final File privateKeyFile, final File publicKeyFile) throws Throwable {
        byte[] publicKeyBytes = null;

        final Class<?> openSSHPrivateKeyUtilClass = keepMyClassForNamesR8("org.bouncycastle.crypto.util.OpenSSHPrivateKeyUtil");
        final Class<?> openSSHPublicKeyUtilClass = keepMyClassForNamesR8("org.bouncycastle.crypto.util.OpenSSHPublicKeyUtil");
        final Class<?> pemObjectClass = keepMyClassForNamesR8("org.bouncycastle.util.io.pem.PemObject");
        final Class<?> asymmetricKeyParameterClass = keepMyClassForNamesR8("org.bouncycastle.crypto.params.AsymmetricKeyParameter");
        final Method encodePublicKeyMethod = openSSHPublicKeyUtilClass.getMethod("encodePublicKey", asymmetricKeyParameterClass);

        if (!privateKeyFile.exists()) {
            final Class<?> ed25519keyPairGeneratorClass = keepMyClassForNamesR8("org.bouncycastle.crypto.generators.Ed25519KeyPairGenerator");
            final Class<?> ed25519keyGenerationParametersClass = keepMyClassForNamesR8("org.bouncycastle.crypto.params.Ed25519KeyGenerationParameters");
            final Class<?> keyGenerationParametersClass = keepMyClassForNamesR8("org.bouncycastle.crypto.KeyGenerationParameters");
            final Class<?> asymmetricCipherKeyPairClass = keepMyClassForNamesR8("org.bouncycastle.crypto.AsymmetricCipherKeyPair");
            final Class<?> pemWriterClass = keepMyClassForNamesR8("org.bouncycastle.util.io.pem.PemWriter");
            final Class<?> pemObjectGeneratorClass = keepMyClassForNamesR8("org.bouncycastle.util.io.pem.PemObjectGenerator");

            final Object keyPairGenerator = ed25519keyPairGeneratorClass.getDeclaredConstructor().newInstance();
            final Constructor<?> keyGenerationParametersConstructor = ed25519keyGenerationParametersClass.getConstructor(SecureRandom.class);
            final Object keyGenerationParameters = keyGenerationParametersConstructor.newInstance(new SecureRandom());

            final Method initMethod = ed25519keyPairGeneratorClass.getMethod("init", keyGenerationParametersClass);
            initMethod.invoke(keyPairGenerator, keyGenerationParameters);

            final Method generateKeyPairMethod = ed25519keyPairGeneratorClass.getMethod("generateKeyPair");
            final Object keyPair = generateKeyPairMethod.invoke(keyPairGenerator);

            final Method encodePrivateKeyMethod = openSSHPrivateKeyUtilClass.getMethod("encodePrivateKey", asymmetricKeyParameterClass);
            final Method getPrivateMethod = asymmetricCipherKeyPairClass.getMethod("getPrivate");
            final byte[] privateKeyBytes = (byte[]) encodePrivateKeyMethod.invoke(null, getPrivateMethod.invoke(keyPair));

            try (final FileWriter fileWriter = new FileWriter(privateKeyFile); final BufferedWriter pemWriter = (BufferedWriter) pemWriterClass.getConstructor(Writer.class).newInstance(fileWriter)) {
                final Constructor<?> pemObjectConstructor = pemObjectClass.getConstructor(String.class, byte[].class);
                final Object pemObject = pemObjectConstructor.newInstance("OPENSSH PRIVATE KEY", privateKeyBytes);
                final Method writeObjectMethod = pemWriterClass.getMethod("writeObject", pemObjectGeneratorClass);
                writeObjectMethod.invoke(pemWriter, pemObject);
            }

            final Method getPublicMethod = asymmetricCipherKeyPairClass.getMethod("getPublic");
            publicKeyBytes = (byte[]) encodePublicKeyMethod.invoke(null, getPublicMethod.invoke(keyPair));
        }

        if (publicKeyBytes != null || !publicKeyFile.exists()) {
            if (publicKeyBytes == null) {
                final Class<?> pemReaderClass = keepMyClassForNamesR8("org.bouncycastle.util.io.pem.PemReader");
                try (final FileReader fileReader = new FileReader(privateKeyFile); final BufferedReader pemReader = (BufferedReader) pemReaderClass.getConstructor(Reader.class).newInstance(fileReader)) {
                    final Class<?> ed25519PrivateKeyParametersClass = keepMyClassForNamesR8("org.bouncycastle.crypto.params.Ed25519PrivateKeyParameters");
                    final Method readPemObjectMethod = pemReaderClass.getMethod("readPemObject");
                    final Object pemObject = readPemObjectMethod.invoke(pemReader);

                    final Method getContentMethod = pemObjectClass.getMethod("getContent");
                    final byte[] privateKeyBlob = (byte[]) getContentMethod.invoke(pemObject);

                    final Method parsePrivateKeyBlobMethod = openSSHPrivateKeyUtilClass.getMethod("parsePrivateKeyBlob", byte[].class);
                    final Object params = parsePrivateKeyBlobMethod.invoke(null, (Object) privateKeyBlob);

                    final Method generatePublicKeyMethod = ed25519PrivateKeyParametersClass.getMethod("generatePublicKey");
                    publicKeyBytes = (byte[]) encodePublicKeyMethod.invoke(null, generatePublicKeyMethod.invoke(params));
                }
            }

            try (final FileWriter fileWriter = new FileWriter(publicKeyFile)) {
                final String publicKeyString = String.format("ssh-ed25519 %s go2sleep@%s\n",
                        Base64.getEncoder().encodeToString(publicKeyBytes), Build.BOARD);
                fileWriter.write(publicKeyString);
            }
        }
    }
}
