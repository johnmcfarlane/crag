#!/usr/bin/env bash
set -e

# init
cd "$( dirname "${BASH_SOURCE[0]}" )"
source script/init.sh

# delete stuff
git clean --force -dX .
