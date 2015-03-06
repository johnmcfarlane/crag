#!/usr/bin/env bash
set -e
set -x

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh
CONFIG="$1"
DEST_DIR="$2"
BINARY_FILENAME=pnacl/Debug/crag.pexe

# build artifacts
./build.sh "$CONFIG"

if diff "$BINARY_FILENAME" "$DEST_DIR""$BINARY_FILENAME" >/dev/null ; then
  echo Change detected

  if [ "$CONFIG" == "Debug" ];
  then
    echo Debug Prep
    script/gdb-prep.sh
  fi

  # process/move newly built files
  script/install.sh "$DEST_DIR"

  # clear Chrome user data dir
  rm -rf tmp
else
  echo No change detected
fi

script/run_chrome.sh
