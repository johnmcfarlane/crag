# Building Crag for Linux

## Introduction

This file details how to build and run [Crag](https://code.google.com/p/crag/) on [Debian](https://www.debian.org/) GNU Linux systems.
It is tested against [Ubuntu 14.04](http://releases.ubuntu.com/trusty/) but should be straight-forward to get working on recent [Debian Jessie](https://www.debian.org/releases/jessie/) desktop releases.

## Setup and Install

1. Install Git and your favorite C++11 compiler:

   `sudo apt-get install git g++`

   Crag is regularly tested against g++ and Clang. Other packages are installed in step 3.

2. Clone the project:

   `git clone https://code.google.com/p/crag/ -b linux`

   (More instructions on cloning Crag can be found [here](https://code.google.com/p/crag/source/checkout).)

3. Run the setup script which installs Debian packages and other dependencies:

   `./crag/linux/setup`

## Build and Run

1. Use cmake to generate a release build Makefile:

   For Raspberry Pi:

   `cmake -DCMAKE_BUILD_TYPE=Release -DCRAG_RPI=1`

   For PC:

   `cmake -DCMAKE_BUILD_TYPE=Release`

2. Compiler and link the program:

   `make`

3. Run:

   `./crag`

## CLion IDE

[CLion](https://www.jetbrains.com/clion/) is a yet-to-be released IDE from [JetBrains](https://www.jetbrains.com/). 
Crag includes tentative Linux support for it.
To try it out:

1. Install a JDK, e.g.:

   `sudo apt-get install openjdk-7-jdk`

2. Download, extract and launch the latest [Early Access Program (EAP)](https://confluence.jetbrains.com/display/CLION/Early+Access+Program) build.

3. Select "Open Project" and direct CLion to the crag directory.

4. Go to run configuration dialog (Run -> Edit Configuration) and set Working Directory to the crag folder.
