# Building Crag for NaCL

## Introduction

This file details how to build and run [Crag](https://github.com/johnmcfarlane/crag) on [Debian](https://www.debian.org/) GNU Linux systems.
It is tested against [Ubuntu 14.04](http://releases.ubuntu.com/trusty/) but should be straight-forward to get working on recent [Debian Jessie](https://www.debian.org/releases/jessie/) desktop releases.

NaCL is work in progress and currently crashes consistently.

## Setup

Run NaCL setup script:

`crag/nacl/setup.sh`

## Run

Run the `run.sh` script which launches a browser and web server:

`./run.sh`

## Debug

After turning on the appropriate debugging flags in Makefile and launching Chrome with the correct flags
as detailed [here](https://developer.chrome.com/native-client/devguide/devcycle/debugging),
launch the debugger:

`crag/nacl/script/gdb.sh`

From in the debugger:

`target remote localhost:4014`

## Clean using make

`make clean CONFIG=Release SDK_NAME=pepper_40`
