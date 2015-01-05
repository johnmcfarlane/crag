NDK_TOOLCHAIN_VERSION := 4.8
#NDK_TOOLCHAIN_VERSION := clang

#APP_ABI := all
APP_ABI := armeabi-v7a

APP_PLATFORM := android-14

#APP_OPTIM := debug
APP_OPTIM := release

APP_CPPFLAGS += -std=c++11
APP_STL := gnustl_static
