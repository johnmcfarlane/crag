#!/usr/bin/env bash
set -e

# system packages
sudo apt-get install libc6:i386 git mercurial automake libtool

# init
LOCAL_DIR="$( dirname "${BASH_SOURCE[0]}" )"
SCRIPT_DIR="$LOCAL_DIR"/script
source "$SCRIPT_DIR"/init.sh

#sudo apt-get install make cmake automake g++-4.8 mercurial libglu1-mesa-dev libode-sp-dev libxi-dev build-essential libpng12-dev

"$SCRIPT_DIR"/setup_nacl.sh
"$SCRIPT_DIR"/setup_ode.sh
"$SCRIPT_DIR"/setup_sdl.sh
"$SCRIPT_DIR"/setup_sdl_image.sh
