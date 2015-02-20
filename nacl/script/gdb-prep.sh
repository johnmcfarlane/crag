#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh

BUILD_DIR="$NACL_DIR"/pnacl/Release
UNSTRIPPED_PEXE="$BUILD_DIR"/crag_unstripped.pexe
UNSTRIPPED_NEXE="$BUILD_DIR"/crag_unstripped.nexe

"$NACL_SDK_ROOT"/toolchain/linux_pnacl/bin/pnacl-translate --allow-llvm-bitcode-input "$UNSTRIPPED_PEXE" -arch x86-64 -o "$UNSTRIPPED_NEXE"
