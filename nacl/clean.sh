#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/script/init.sh

# delete stuff
cd "$LOCAL_DIR"
git clean --force -dX .
