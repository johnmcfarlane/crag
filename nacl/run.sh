#!/usr/bin/env bash
set -e

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
CHROME_DIR=/opt/google/chrome/

# where to put all of Chrome's files
CHROME_DATA_DIR=$LOCAL_DIR/tmp
mkdir -p $CHROME_DATA_DIR
echo Chrome user are in $CHROME_DATA_DIR

# chrome environment variables
export NACL_PLUGIN_DEBUG=1
export NACL_SRPC_DEBUG=[255]
export NACLVERBOSITY=[255]

export NACL_EXE_STDERR="$LOCAL_DIR"/nacl_stderr.log
export NACL_EXE_STDOUT="$LOCAL_DIR"/nacl_stdout.log
export NACLLOG=~/nacl.log

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$CHROME_DIR"

# launch chrome directly
"$CHROME_DIR"/chrome --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --args --enable-logging --vmodule=ppb*=4 http://localhost:8000/ &

python -m SimpleHTTPServer
