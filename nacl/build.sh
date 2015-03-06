#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh

# build artifacts
# Debug or Release
make -j"$NCPUS" CONFIG=$1 SDK_NAME=$SDK_NAME
