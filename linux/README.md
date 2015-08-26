# Building Crag for GNU/Linux

## Introduction

This file details how to build and run [Crag](https://github.com/johnmcfarlane/crag) on PCs running [Debian](https://www.debian.org/) GNU Linux.
It is tested against [Ubuntu 14.04](http://releases.ubuntu.com/trusty/) but should be straight-forward to get working on recent [Debian Jessie](https://www.debian.org/releases/jessie/) desktop releases.

See [README-rpi.md] for instructions targeted at Raspberry Pi.

## Setup and Install

1. Install the necessary packages:

   ```
   sudo apt-get install git mercurial g++ cmake automake libode-sp-dev libglu1-mesa-dev xorg-dev libsdl2-dev libsdl2-image-dev libode-sp-dev
   ```

   Crag is regularly tested against G++ 4.9 and Clang 3.6.

2. Clone the project using the instructions in [the main README.md](../README.md).

## Run

1. Use cmake to generate a release build Makefile:

   ```
   cmake -DCMAKE_BUILD_TYPE=Release
   ```

2. Compile and link the program:

   ```
   make
   ```

   (You can speed up compilation on multicore systems using the [jobs](https://www.gnu.org/software/make/manual/html_node/Parallel.html) option.)

3. Run:

   ```
   ./crag
   ```

## Debug

1. Build and run as above but with the `Debug` build type and no limit on core dump size:

   ```
   cmake -DCMAKE_BUILD_TYPE=Debug
   make
   ulimit -c unlimited
   ./crag
   ```

2. Following a crash, a core file, `core`, will appear in the working directory.

   ```
   gdb ./crag core -batch -ex bt
   ```

## Profile

1. Preparation:
   ```
   sudo apt-get install valgrind kcachegrind
   ```

1. In CMakeLists.txt, add "-g" switch to CMAKE_CXX_FLAGS_RELEASE:
   ```
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -fno-rtti -Ofast -g")
   ```

   and add:
   ```
   set(CMAKE_EXE_LINKER_FLAGS "-g")
   ```

1. Then:
   ```
   cd crag/linux
   cmake -DCMAKE_BUILD_TYPE=Profile
   valgrind --tool=callgrind ./crag
   kcachegrind callgrind.out.XXXXX &
   ```

## CLion IDE

[CLion](https://www.jetbrains.com/clion/) is a yet-to-be released IDE from [JetBrains](https://www.jetbrains.com/).
Crag includes tentative Linux support for it.
To try it out:

1. Install a JDK, e.g.:

   ```
   sudo apt-get install openjdk-7-jdk
   ```

2. Download, extract and launch the latest [Early Access Program (EAP)](https://confluence.jetbrains.com/display/CLION/Early+Access+Program) build.

3. Select "Open Project" and direct CLion to the `crag/linux` directory.

4. Go to run configuration dialog (Run -> Edit Configuration) and set Working Directory to the `crag/linux` folder.

### Troubleshooting

If you receive a build message complaining that a file "has been modified since the precompiled header 'pch.h.gch/.c++' was built",
you will need to select the Clean option from the Run menu.
