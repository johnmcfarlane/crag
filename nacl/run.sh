#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
source "$LOCAL_DIR"/script/init.sh
CHROME_DIR=/opt/google/chrome/

export NACL_PLUGIN_DEBUG=1
export NACL_SRPC_DEBUG=[255]
export NACLVERBOSITY=[255]

export NACL_EXE_STDERR=~/nacl_stderr.log
export NACL_EXE_STDOUT=~/nacl_stdout.log
export NACLLOG=~/nacl.log

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$CHROME_DIR"

"$CHROME_DIR"/chrome index.html --args --enable-logging --no-sandbox --vmodule=ppb*=4
#python -m SimpleHTTPServer
