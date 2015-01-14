#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/init.sh
SDL_DIR="$NACL_DIR"/SDL

# get SDL
hg clone http://hg.libsdl.org/SDL "$SDL_DIR" || true
cd "$SDL_DIR"
hg pull http://hg.libsdl.org/SDL
hg update

# configure SDL
export CFLAGS="$CFLAGS -I$NACL_SDK_ROOT/include -I$NACL_SDK_ROOT/include/pnacl"
export CC="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-clang"
export AR="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export LD="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export LDFLAGS="-L$NACL_SDK_ROOT/lib"
export LIBS="-lm"
export RANLIB="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ranlib"
export SDL2_CONFIG="$NACL_SDK_ROOT"/bin/sdl2-config

#./autogen.sh
./configure --host=pnacl --prefix "$NACL_DIR"
make -j4
make install

#export CFLAGS="$CFLAGS -I$NACL_SDK_ROOT/include -I$NACL_SDK_ROOT/include/pnacl -I$NACL_SDK_ROOT/ports/include"
#export LDFLAGS="-L$NACL_SDK_ROOT/ports/lib/newlib_pnacl/Release -L$NACL_SDK_ROOT/lib -L$NACL_SDK_ROOT/lib/pnacl/Release"
#
#export SDL2_CONFIG="$NACL_SDK_ROOT"/bin/sdl2-config
#make CONFIG="Release"
