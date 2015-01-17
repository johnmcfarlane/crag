#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/init.sh
SDL_IMAGE_DIR="$NACL_DIR"/SDL_image

# get SDL_image
hg clone http://hg.libsdl.org/SDL_image "$SDL_IMAGE_DIR" || true
cd "$SDL_IMAGE_DIR"
hg pull http://hg.libsdl.org/SDL_image
hg update

# configure SDL_image
export CC="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-clang"
export AR="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar" 
export LD="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export LDFLAGS="-L$NACL_SDK_ROOT/lib/pnacl/Release"
export RANLIB="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ranlib"
export SDL2_CONFIG="$NACL_DIR"/bin/sdl2-config

./autogen.sh
./configure --host=pnacl --disable-png --prefix "$NACL_DIR"

make CONFIG="Release" -j"$NCPUS" || true

# TODO: fix so make doesn't fail
#make install
cp "$SDL_IMAGE_DIR"/.libs/libSDL2_image.a "$NACL_DIR"/lib
cp "$SDL_IMAGE_DIR"/.libs/libSDL2_image.la "$NACL_DIR"/lib
cp "$SDL_IMAGE_DIR"/SDL_image.h "$NACL_DIR"/include/SDL2
