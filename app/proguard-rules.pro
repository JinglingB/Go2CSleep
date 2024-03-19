# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

-repackageclasses
-overloadaggressively
-optimizationpasses 5

-keep,allowoptimization class big.pimpin.go2sleephoe.Ed25519Keygen {
    static void generateAndWriteEd25519KeyPair(java.io.File, java.io.File);
}
-keep,allowoptimization class org.bouncycastle.crypto.KeyGenerationParameters { *; }
-keep,allowoptimization class org.bouncycastle.crypto.generators.Ed25519KeyPairGenerator { *; }
-keep,allowoptimization class org.bouncycastle.crypto.params.Ed25519KeyGenerationParameters { *; }
-keep,allowoptimization class org.bouncycastle.crypto.params.Ed25519PrivateKeyParameters { *; }
-keep,allowoptimization class org.bouncycastle.crypto.util.OpenSSHPrivateKeyUtil { *; }
-keep,allowoptimization class org.bouncycastle.crypto.util.OpenSSHPublicKeyUtil { *; }
-keep,allowoptimization class org.bouncycastle.crypto.params.AsymmetricKeyParameter { *; }
-keep,allowoptimization class org.bouncycastle.crypto.AsymmetricCipherKeyPair { *; }
-keep,allowoptimization class org.bouncycastle.util.io.pem.PemWriter { *; }
-keep,allowoptimization class org.bouncycastle.util.io.pem.PemReader { *; }
-keep,allowoptimization class org.bouncycastle.util.io.pem.PemObject { *; }
-keep,allowoptimization class org.bouncycastle.util.io.pem.PemObjectGenerator { *; }
