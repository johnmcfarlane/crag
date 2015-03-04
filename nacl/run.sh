#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh

"$SCRIPT_DIR"/run_chrome.sh &
"$SCRIPT_DIR"/run_server.sh
