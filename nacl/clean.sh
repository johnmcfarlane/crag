#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
cd "$LOCAL_DIR"/script
source ./init.sh

# delete stuff
cd "$LOCAL_DIR"
git clean --force -dX .
