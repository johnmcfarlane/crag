#!/usr/bin/env bash
set -e

# cd crag/linux
cd "$( dirname "${BASH_SOURCE[0]}" )"

################################################################################
# SDL

./script/setup_sdl $@

################################################################################
# SDL_image

./script/setup_sdl_image
