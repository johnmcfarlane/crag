# Building Crag for Raspberry Pi

## Introduction

This file details how to build and run [Crag](https://github.com/johnmcfarlane/crag) on [Raspberry Pi](http://www.raspberrypi.org/) running [Raspbian](http://www.raspbian.org/) GNU Linux. Tips on configuring the system for best performance can be found in the [SDL documentation](https://hg.libsdl.org/SDL/file/tip/docs/README-raspberrypi.md).

## Setup and Install

1. Install a C++11 compiler. GCC 4.8 or above is a good choice. This is a slightly tricky task on Raspbian. Based on instructions [here](http://www.raspberrypi.org/forums/viewtopic.php?f=33&t=22938) try:

   ```
   sudo apt-get install git g++-4.8
   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
   sudo update-alternatives --config c++
   ```

   This enables you to choose between 4.8 and the pre-installed compiler.

2. Some other packages that you will likely need:

   `sudo apt-get install git mercurial cmake automake libode-sp-dev`

3. Clone the project:

   `git clone https://github.com/johnmcfarlane/crag.git`

4. Run the setup script which installs Debian packages and other dependencies:

   ```
   cd crag
   linux/setup.sh --host=arm-raspberry-linux-gnueabihf --target=arm-raspberry-linux-gnueabihf --disable-video-x11
   ```

## Build and Run

1. Use cmake to generate a release build Makefile:

   `cmake -DCMAKE_BUILD_TYPE=Release -DCRAG_RPI=1`

2. Compile and link the program:

   `make`

3. Run:

   `./crag`

