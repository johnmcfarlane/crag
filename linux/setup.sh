#!/usr/bin/env bash
set -e

# cd crag/linux
cd "$( dirname "${BASH_SOURCE[0]}" )"

ln -sf ../assets .

################################################################################
# SDL

./script/setup_sdl $@

################################################################################
# SDL_image

./script/setup_sdl_image
