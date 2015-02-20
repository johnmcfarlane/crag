#!/usr/bin/env bash
set -e

LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
cd "$LOCAL_DIR"/..
ls

python -m SimpleHTTPServer 8000
