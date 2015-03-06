#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh

SOURCE_DIR="$NACL_DIR"
DEST_DIR="$1"

cp "$SOURCE_DIR"/background.js "$DEST_DIR"
cp "$SOURCE_DIR"/common.js "$DEST_DIR"
cp "$SOURCE_DIR"/index.html "$DEST_DIR"
cp "$SOURCE_DIR"/manifest.json "$DEST_DIR"

RELEASE_SUBDIR="pnacl/Release"
mkdir -p "$DEST_DIR"/"$RELEASE_SUBDIR"
cp "$SOURCE_DIR"/"$RELEASE_SUBDIR"/crag.nmf "$DEST_DIR"/"$RELEASE_SUBDIR"
cp "$SOURCE_DIR"/"$RELEASE_SUBDIR"/crag.pexe "$DEST_DIR"/"$RELEASE_SUBDIR"

# debug files
cp "$SOURCE_DIR"/"$RELEASE_SUBDIR"/crag_unstripped.bc "$DEST_DIR"/"$RELEASE_SUBDIR"

ASSETS_SUBDIR="assets"
cp -rfL "$SOURCE_DIR/$ASSETS_SUBDIR"/ "$DEST_DIR"
