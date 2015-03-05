#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh
DEST_DIR="$1"
BINARY_FILENAME=pnacl/Release/crag.pexe

# build artifacts
./build.sh

if diff "$BINARY_FILENAME" "$DEST_DIR""$BINARY_FILENAME" >/dev/null ; then
  echo No change detected
else
  echo Change detected
  rm -rf tmp
  script/install.sh "$DEST_DIR"
fi

script/run_chrome.sh
