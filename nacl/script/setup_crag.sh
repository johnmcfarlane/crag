#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/init.sh

# build
make -j"$NCPUS"

# help
echo run '"python -m SimpleHTTPServer"'
