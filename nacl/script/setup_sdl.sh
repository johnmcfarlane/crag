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
export RANLIB="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ranlib"

"$SDL_DIR"/configure --host=pnacl --prefix "$NACL_DIR"
./build-scripts/naclbuild.sh "$NACL_SDK_ROOT"
make -j$NCPUS CFLAGS="$CFLAGS -I./include"
make install
