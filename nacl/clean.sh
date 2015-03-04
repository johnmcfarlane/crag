#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh

# remove artifacts
make clean -j"$NCPUS" CONFIG=Release SDK_NAME=$SDK_NAME
