#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh

# build artifacts
make -j"$NCPUS" CONFIG=Release $@ SDK_NAME=$SDK_NAME
