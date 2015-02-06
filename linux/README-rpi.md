# Building Crag for Raspberry Pi

## Introduction

This file details how to build and run [Crag](https://code.google.com/p/crag/) on [Raspberry Pi](http://www.raspberrypi.org/) running [Raspbian](http://www.raspbian.org/) GNU Linux. Tips on configuring the system for best performance can be found in the [SDL documentation](https://hg.libsdl.org/SDL/file/tip/docs/README-raspberrypi.md).

## Setup and Install

1. Install Git and your favorite C++11 compiler:

   `sudo apt-get install git g++`

   Crag is regularly tested against g++ and Clang. Other packages are installed in step 3.

2. Clone the project:

   `git clone https://code.google.com/p/crag/ -b rpi`

   (More instructions on cloning Crag can be found [here](https://code.google.com/p/crag/source/checkout).)

3. Run the setup script which installs Debian packages and other dependencies:

   ```
   cd crag
   ./linux/setup.sh --host=arm-raspberry-linux-gnueabihf --target=arm-raspberry-linux-gnueabihf --disable-video-x11
   ```

## Build and Run

1. Use cmake to generate a release build Makefile:

   `cmake -DCMAKE_BUILD_TYPE=Release -DCRAG_RPI=1`

2. Compile and link the program:

   `make`

3. Run:

   `./crag`

