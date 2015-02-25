#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh

BUILD_DIR="$NACL_DIR"/pnacl/Release
UNSTRIPPED_PEXE="$BUILD_DIR"/crag_unstripped.pexe
UNSTRIPPED_NEXE="$BUILD_DIR"/crag_unstripped.nexe

"$NACL_SDK_ROOT"/toolchain/linux_x86_newlib/bin/x86_64-nacl-gdb "$UNSTRIPPED_NEXE"
