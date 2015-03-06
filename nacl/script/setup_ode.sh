#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh
ODE_DIR="$NACL_DIR"/ode

# get ODE
hg clone https://bitbucket.org/odedevs/ode "$ODE_DIR" || true
cd "$ODE_DIR"
hg pull https://bitbucket.org/odedevs/ode
hg update

# configure ODE
export CFLAGS="-fno-exceptions -fno-rtti -O3"
export CXXFLAGS="-fno-exceptions -fno-rtti -O3"
export CC="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-clang"
export CXX="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-clang"
export AR="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export LD="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ar"
export RANLIB="$NACL_SDK_ROOT/toolchain/linux_pnacl/bin/pnacl-ranlib"

./bootstrap
./configure --host=nacl --enable-single-precision --disable-demos --disable-asserts --disable-libccd --prefix="$NACL_DIR"
make -j"$NCPUS"
make install
