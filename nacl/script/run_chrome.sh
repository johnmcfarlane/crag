#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"/..
source script/init.sh

# where to put all of Chrome's files
CHROME_DATA_DIR="$NACL_DIR"/tmp
CHROME_DIR=/opt/google/chrome/

mkdir -p $CHROME_DATA_DIR
echo Chrome user files are in $CHROME_DATA_DIR

CRAG_URL=http://localhost:8000

# chrome environment variables
export NACL_PLUGIN_DEBUG=1
export NACL_SRPC_DEBUG=0
export NACLVERBOSITY=0

export NACL_EXE_STDERR="$NACL_DIR"/stderr.log
rm -f "$NACL_EXE_STDERR"

export NACL_EXE_STDOUT="$NACL_DIR"/stdout.log
rm -f "$NACL_EXE_STDOUT"

#export NACLLOG="$NACL_DIR"/nacl.log
rm -f "$NACLLOG"

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$CHROME_DIR"

# launch chrome directly
"$CHROME_DIR"/chrome "$CRAG_URL" --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --enable-logging=stderr --vmodule=ppb*=4
#"$CHROME_DIR"/chrome "$CRAG_URL" --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --enable-logging=stderr --vmodule=ppb*=4 --enable-nacl-debug --enable-nacl --disable-hang-monitor
