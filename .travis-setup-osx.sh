#!/usr/bin/env bash

brew install libtool

ln -s `which glibtoolize` libtoolize
export PATH=$(pwd):$PATH
