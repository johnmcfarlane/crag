#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh

# download NaCL SDK and unzip to crag/nacl
DOWNLOAD_DIR="$NACL_DIR"
mkdir -p "$DOWNLOAD_DIR"
wget --no-clobber --directory-prefix="$DOWNLOAD_DIR" http://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip
unzip -o "$DOWNLOAD_DIR"/nacl_sdk.zip -d "$NACL_DIR"

# install SDK version
"$NACL_DIR"/nacl_sdk/naclsdk update "$SDK_NAME"
