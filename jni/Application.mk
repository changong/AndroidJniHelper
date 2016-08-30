APP_STL := gnustl_static
NDK_TOOLCHAIN_VERSION=clang

APP_ABI := x86 armeabi armeabi-v7a

APP_CPPFLAGS := -frtti -DCC_ENABLE_CHIPMUNK_INTEGRATION=1 -std=c++11 -fsigned-char
APP_LDFLAGS := -latomic