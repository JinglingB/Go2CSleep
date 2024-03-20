# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# https://github.com/bk138/multivnc/blob/master/android/app/src/main/cpp/CMakeLists.txt

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("go2sleephoe")

set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# BoringSSL
find_package(boringssl REQUIRED CONFIG)
get_target_property(OPENSSL_CRYPTO_LIBRARY boringssl::crypto_static IMPORTED_LOCATION)
get_target_property(OPENSSL_SSL_LIBRARY boringssl::ssl_static IMPORTED_LOCATION)
get_target_property(OPENSSL_INCLUDE_DIR boringssl::ssl_static INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${OPENSSL_INCLUDE_DIR})

# libssh2
set(BUILD_EXAMPLES OFF CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")
set(CRYPTO_BACKEND "OpenSSL" CACHE STRING "")
set(ENABLE_ZLIB_COMPRESSION ON CACHE BOOL "")
set(LIBSSH2_NO_DEPRECATED ON CACHE BOOL "")
string(APPEND CMAKE_C_FLAGS " -DLIBSSH2_NO_MD5 -DLIBSSH2_NO_MD5_PEM -DLIBSSH2_NO_HMAC_RIPEMD -DLIBSSH2_NO_DSA -DLIBSSH2_NO_BLOWFISH -DLIBSSH2_NO_RC4 -DLIBSSH2_NO_CAST -DLIBSSH2_NO_3DES")
set(libssh2_src_DIR ${PROJECT_SOURCE_DIR}/../../../../extern/libssh2)
set(libssh2_build_DIR ${CMAKE_BINARY_DIR}/libssh2)
add_subdirectory(${libssh2_src_DIR} ${libssh2_build_DIR})
include_directories(${libssh2_src_DIR}/include ${libssh2_build_DIR}/src)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.c)

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        libssh2)
