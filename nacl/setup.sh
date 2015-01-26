#!/usr/bin/env bash
set -e

# orient
SCRIPT_DIR="$( dirname "${BASH_SOURCE[0]}" )"/script

# system packages
sudo apt-get install libc6:i386 git mercurial automake libtool

# init
"$SCRIPT_DIR"/setup_nacl.sh
"$SCRIPT_DIR"/setup_ode.sh
"$SCRIPT_DIR"/setup_sdl.sh
"$SCRIPT_DIR"/setup_sdl_image.sh
"$SCRIPT_DIR"/setup_crag.sh
