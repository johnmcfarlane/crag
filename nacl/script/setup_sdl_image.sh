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
export CFLAGS="-I$NACL_DIR/include/SDL2 -I$NACL_SDK_ROOT/include -I$NACL_SDK_ROOT/include/pnacl -I$NACL_SDK_ROOT/ports/include"
export CC="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-clang"
export AR="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar" 
export LD="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export LDFLAGS="-L$NACL_DIR/lib -L$NACL_SDK_ROOT/ports/lib/newlib_pnacl/Release -L$NACL_SDK_ROOT/lib -L$NACL_SDK_ROOT/lib/pnacl -L$NACL_SDK_ROOT/lib/pnacl/Release"
export LIBS="-lm -lppapi_gles2 -lppapi_simple -lSDL2 -lppapi_gles2"
export RANLIB="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ranlib"
export SDL2_CONFIG="$NACL_SDK_ROOT"/bin/sdl2-config

#./autogen.sh
./configure --host=pnacl --prefix "$NACL_DIR"

make CONFIG="Release" || true

# TODO: fix SDL_image WRT PNaCL
#make install
cp "$SDL_IMAGE_DIR"/.libs/libSDL2_image.a "$NACL_DIR"/lib
cp "$SDL_IMAGE_DIR"/.libs/libSDL2_image.la "$NACL_DIR"/lib
cp "$SDL_IMAGE_DIR"/SDL_image.h "$NACL_DIR"/include/SDL2
