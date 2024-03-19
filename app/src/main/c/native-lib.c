#include "ssh_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/cdefs.h>

#include "libssh2_config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <fcntl.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_POLL
#include <poll.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <netinet/tcp.h>

#include <jni.h>
#include <libssh2.h>

#define	nitems(x) (sizeof((x)) / sizeof((x)[0]))

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
// By Jay Sullivan, taken from https://stackoverflow.com/a/61960339
static int connect_with_timeout(int sockfd, const struct sockaddr *addr, const socklen_t addrlen, const unsigned int timeout_ms)
{
    int rc = 0;

    // Set O_NONBLOCK
    const int sockfd_flags_before = fcntl(sockfd, F_GETFL, 0);
    if (sockfd_flags_before < 0)
        return -1;
    if (fcntl(sockfd, F_SETFL, sockfd_flags_before | O_NONBLOCK) < 0)
        return -1;
    // This one-time 'loop' just lets us 'break' to get out of it
    do
    {
        // Start connecting (asynchronously)
        if (connect(sockfd, addr, addrlen) >= 0)
            break;

        // Did connect return an error? If so, we'll fail.
        if (errno != EWOULDBLOCK && errno != EINPROGRESS)
        {
            rc = -1;
            break;
        }

        // Otherwise, asynchronous connect has begun
        // We'll now wait for one of (A) timeout expired, or (B) connection completed.
        // Set a deadline timestamp 'timeout' ms from now. (Needed b/c poll can be interrupted.)
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) < 0)
        {
            rc = -1;
            break;
        }
        const struct timespec deadline = {.tv_sec = now.tv_sec,
                                          .tv_nsec = now.tv_nsec + timeout_ms * 1000000l};
        // We'll repeatedly poll in 500ms intervals. (Needed b/c poll won't detect POLLHUP if it happens during poll.)
        do
        {
            // Calculate how long until the deadline
            if (__predict_false(clock_gettime(CLOCK_MONOTONIC, &now) < 0))
            {
                rc = -1;
                break;
            }
            const int ms_until_deadline = (int)((deadline.tv_sec - now.tv_sec) * 1000l + (deadline.tv_nsec - now.tv_nsec) / 1000000l);
            // (A) If the timeout has expired, exit.
            if (ms_until_deadline < 0)
            {
                rc = 0;
                break;
            }
            // Perform the poll
            struct pollfd pfds[] = {{.fd = sockfd, .events = POLLHUP | POLLERR | POLLOUT}};
            rc = poll(pfds, 1, MIN(ms_until_deadline + 1, 500));
            // (B) If the connection has completed, check to see whether it succeeded or failed, then exit.
            if (rc > 0 && __predict_true(pfds[0].revents > 0))
            {
                int error = 0;
                socklen_t len = sizeof(error);
                const int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                if (retval == 0)
                    errno = error;
                if (error != 0)
                    rc = -1;
            }
        }
        // If poll had a 500ms-timeout, keep going.
        while (rc == 0);
        // Did poll timeout? If so, fail.
        if (rc == 0)
        {
            errno = ETIMEDOUT;
            rc = -1;
        }
    } while (0);
    // Restore original O_NONBLOCK state
    if (__predict_false(fcntl(sockfd, F_SETFL, sockfd_flags_before) < 0))
        return -1;
    // Success
    return rc;
}
#pragma clang diagnostic pop

static void waitsocket(libssh2_socket_t socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout = {
        .tv_sec = 5,
        .tv_usec = 0
    };
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;

    FD_ZERO(&fd);
    FD_SET(socket_fd, &fd);

    /* now make sure we wait in the correct direction */
    const int dir = libssh2_session_block_directions(session);
    if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;
    if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    select((int)(socket_fd + 1), readfd, writefd, NULL, &timeout);
}

void JNICALL Java_ssh2_exec(JNIEnv *env, __unused const jobject this, const jstring Java_commandline, const jstring Java_id_ed25519_path, const jstring Java_id_ed25519_pub_path)
{
    libssh2_socket_t sock;
    struct sockaddr_in sin;
    int rc;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;

    const char *const commandline = (*env)->GetStringUTFChars(env, Java_commandline, NULL);
    const char *const id_ed25519_path = (*env)->GetStringUTFChars(env, Java_id_ed25519_path, NULL);
    const char *const id_ed25519_pub_path = (*env)->GetStringUTFChars(env, Java_id_ed25519_pub_path, NULL);

    /* Create a session instance */
    session = libssh2_session_init();
    if (__predict_false(session == NULL))
        return;

    const char *const crypt_ciphers = "aes128-gcm@openssh.com,aes256-gcm@openssh.com,aes128-ctr,aes128-cbc,aes192-ctr,aes192-cbc,aes256-ctr,aes256-cbc,rijndael-cbc@lysator.liu.se,arcfour128,arcfour,3des-cbc";
    libssh2_session_method_pref(session, LIBSSH2_METHOD_HOSTKEY, "ssh-ed25519,ssh-rsa,ssh-dss");
    libssh2_session_method_pref(session, LIBSSH2_METHOD_CRYPT_CS, crypt_ciphers);
    libssh2_session_method_pref(session, LIBSSH2_METHOD_CRYPT_SC, crypt_ciphers);

    libssh2_session_set_timeout(session, 5000);
    libssh2_session_set_blocking(session, 0);

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (__predict_false(sock == LIBSSH2_INVALID_SOCKET))
        return;
    const int opt = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, &opt, sizeof(opt));
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

    sin.sin_family = AF_INET;
    if (__predict_false(INADDR_NONE == (sin.sin_addr.s_addr = inet_addr(SSH_HOSTNAME))))
        return;
    sin.sin_port = htons(SSH_PORT);
    if (connect_with_timeout(sock, (struct sockaddr *)(&sin), sizeof(sin), 5000) < 0)
        return;

    while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    if (rc)
        return;

    while((rc = libssh2_userauth_publickey_fromfile(session, SSH_USERNAME,
                                                    id_ed25519_pub_path, id_ed25519_path,
                                                    "")) == LIBSSH2_ERROR_EAGAIN);
    if (rc)
        return;

    do
    {
        channel = libssh2_channel_open_session(session);
        if (channel ||
            libssh2_session_last_error(session, NULL, NULL, 0) !=
            LIBSSH2_ERROR_EAGAIN)
            break;
        waitsocket(sock, session);
    } while (1);
    if (!channel)
        return;

    while ((rc = libssh2_channel_exec(channel, commandline)) == LIBSSH2_ERROR_EAGAIN)
        waitsocket(sock, session);

    // skip cleanup
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, __unused void *reserved)
{
    JNIEnv *env;

    if (__predict_false(libssh2_init(0) != 0))
        return JNI_ERR;

    if (__predict_false((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK))
        return JNI_ERR;

    const jclass c = (*env)->FindClass(env, "big/pimpin/go2sleephoe/BaseSshActivity");
    if (__predict_true(c != NULL)) {
        static const JNINativeMethod methods[] = {
            {"ssh2_exec", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", Java_ssh2_exec},
        };
        (*env)->RegisterNatives(env, c, methods, nitems(methods));
    }

    return JNI_VERSION_1_6;
}
