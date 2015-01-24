#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/init.sh

# build
echo make -j"$NCPUS" CONFIG=Release $@ SDK_NAME=$SDK_NAME
make -j"$NCPUS" CONFIG=Release $@ SDK_NAME=$SDK_NAME
