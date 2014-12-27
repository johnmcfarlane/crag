# Building Crag for Linux

## Introduction

These notes should help build and run [Crag](../README.md) on [Debian](https://www.debian.org/) GNU Linux systems.
It is tested against [Debian Jessie](https://www.debian.org/releases/jessie/) but should be straight-forward to get working on recent [Ubuntu](http://ubuntu.com/) desktop releases.

Non-Debian distros should be able to support Crag with the additional caveat that some instructions on this page rely on `apt-get` to install packages. Proceed accordingly.

## Setup and Install

Install Git and clone the project:
```
sudo apt-get install git
git clone https://code.google.com/p/crag/ -b linux
```
(More instructions on cloning Crag can be found [here](https://code.google.com/p/crag/source/checkout).)

Run the setup script which installs Debian packages and other dependencies:
```
cd crag
./linux/setup
```

## Build and Run

```
cmake -DCMAKE_BUILD_TYPE=Release
make
./crag
```