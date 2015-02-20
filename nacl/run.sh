#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
SCRIPT_DIR="$LOCAL_DIR"/script

"$SCRIPT_DIR"/run_chrome.sh &
"$SCRIPT_DIR"/run_server.sh
