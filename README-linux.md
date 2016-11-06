# Building Crag for GNU/Linux

## Introduction

### Distributions

Crag is known to run on amd64 builds of 
[Debian Jessie](https://www.debian.org/distrib/), [Fedora 24](https://getfedora.org/en/workstation/),
[Ubuntu 14.04](http://releases.ubuntu.com/trusty/), [Ubuntu 16.04](http://releases.ubuntu.com/16.04.1/) and on
[Raspberry Pi](http://www.raspberrypi.org/) builds of [Raspbian Jessie](https://www.raspberrypi.org/downloads/raspbian/).

### Compilers

Crag requires G++ 4.9 or Clang++ 3.5.

## Preparation

1. Install the necessary packages:

   On Debian / Ubuntu:

   ```
   sudo apt-get install automake cmake libglu1-mesa-dev libtool
   ```

   On Raspbian:
   ```
   sudo apt-get install automake libtool
   ```

   On Fedora:
   ```
   sudo yum install automake gcc-c++ cmake libtool mesa-libGL-devel mesa-libGLU-devel
   ```

2. Clone the project:

   ```
   git clone https://github.com/johnmcfarlane/crag.git
   ```

## Run

1. Use cmake to generate a release build Makefile:

   ```
   cd crag
   cmake -DCMAKE_BUILD_TYPE=Release
   ```

2. Compile and link the program:

   ```
   make
   ```

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
