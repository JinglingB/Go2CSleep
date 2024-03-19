A simple Android application to run commands over SSH. This is meant for fully automated usage; there is no UI to change settings. Each (Android) Activity runs a command - the main Activity (i.e. the one a launcher will run) runs a command to put a Windows PC to sleep, hence the name. To create more commands to run, you need to extend `BaseSshActivity` with a command and add the new Activity to AndroidManifest.xml. Changing the destination hostname and port requires recompiling the C library.

This replaces my [Go2Sleep](https://github.com/JinglingB/Go2Sleep). Here, the pure-Java SSHJ library (with the pure-Java Bouncy Castle providing its crypto) is swapped out with the C [libssh2](https://libssh2.org/) library (its crypto is provided by the C++ & ASM BoringSSL).
While SSHJ is plenty fast on my non-cheapo Android devices, it is noticeably slow at connecting on the ARMv8 multicore Android device I wrote Go2Sleep for. It's more of a Bouncy Castle thing - while an amazing library (I continue to use it here to generate the Ed25519 keypair), it can't utilise the weak processors' support for NEON and hardware-accelerated AES instructions. The difference in speed between the SSHJ/Bouncy Castle Go2Sleep and this libssh2/BoringSSL version is like night and day.

You'll need the NDK installed.

1. Make sure this repository is recursively cloned: `git clone --recursive https://github.com/JinglingB/Go2CSleep.git`

2. Set your desired hostname and port in ssh_config.h.template and move that to ssh_config.h

3. Edit MainActivity.java to set the command that will be executed on the remote PC via SSH

4. Build and install on your Android device

5. Run the program. It won't work.

6. Connect via USB debugging and run `adb exec-out cat /sdcard/Android/data/big.pimpin.go2sleephoe/files/id_ed25519.pub | clip` and paste into authorized_keys / administrators_authorized_keys

7. Run the program again. It should work.

Notes:

* This is more geared towards connecting via SSH to a computer on the same LAN, `TCP_NODELAY` is set

* build.gradle will have to be modified in order to build for more platforms than just ARMv7

* A five-second timeout is applied where possible (see above)

* This eschews security a little in favor of speed:

    * Known-hosts verification is not performed

    * The stack protector is turned off for the C code

    * AES128 is favored over AES256 for encryption
    
* There is no disconnection or clean-up performed - this program is intended to exit after the command has been launched and leave the clean-up to the OS

    * Furthermore, because this doesn't stick around, it's very possible your SSH server will terminate the command quickly before it's completed. The remaining code from [ssh2_exec.c](https://libssh2.org/examples/ssh2_exec.html) can be added in order to resolve this.

These things are personal preference, but you should be aware of them should you utilize the code. However, changing these things and to use the more-secure defaults is trivial to achieve.
    
