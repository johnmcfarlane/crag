#!/usr/bin/env bash
set -e

# cd crag/linux
cd "$( dirname "${BASH_SOURCE[0]}" )"

sudo apt-get install cmake mercurial libode-sp-dev libglu1-mesa-dev xorg-dev

################################################################################
# SDL

./script/setup_sdl

################################################################################
# SDL_image

./script/setup_sdl_image
