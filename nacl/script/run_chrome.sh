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

# chrome environment variables
export NACL_PLUGIN_DEBUG=1
export NACL_SRPC_DEBUG=[255]
export NACLVERBOSITY=[255]

export NACL_EXE_STDERR="$NACL_DIR"/stderr.log
export NACL_EXE_STDOUT="$NACL_DIR"/stdout.log
#export NACLLOG="$NACL_DIR"/nacl.log

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH":"$CHROME_DIR"

rm -f "$NACL_EXE_STDERR"
rm -f "$NACL_EXE_STDOUT"
rm -f "$NACLLOG"

# launch chrome directly
"$CHROME_DIR"/chrome http://localhost/~john/crag/ --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --args --enable-logging --vmodule=ppb*=4
#"$CHROME_DIR"/chrome http://localhost/~john/crag/ --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --args --enable-logging --vmodule=ppb*=4 --enable-nacl-debug --enable-nacl --disable-hang-monitor
#"$CHROME_DIR"/chrome http://localhost:8000/ --user-data-dir="$CHROME_DATA_DIR" --test-type --no-sandbox --args --enable-logging --vmodule=ppb*=4
