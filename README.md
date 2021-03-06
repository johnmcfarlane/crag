# [Crag](https://github.com/johnmcfarlane/crag)

[![Build Status](https://travis-ci.org/johnmcfarlane/crag.svg?branch=develop)](https://travis-ci.org/johnmcfarlane/crag)
[![Build status](https://ci.appveyor.com/api/projects/status/b1x763k4ydpiwyuu/branch/develop?svg=true)](https://ci.appveyor.com/project/johnmcfarlane/crag/branch/develop)

![Screenshot](http://john.mcfarlane.name/projects/crag.png)

## Introduction

Crag is a personal programming project developed with the aim of exploring and demonstrating a number of programming techniques.
It takes the form of an interactive 3D simulation featuring a real-time fractal landscape generator and ray-cast sphere renderer.
It is written mostly in C++ and is designed to be as portable as possible, using free libraries and tools including:

- [SDL](http://www.libsdl.org/) & [SDL_image](https://www.libsdl.org/projects/SDL_image/)
- [ODE](http://www.ode.org/)
- [OpenGL](https://www.khronos.org/opengl/) | [OpenGL ES](https://www.khronos.org/opengles/)

## Download

Infrequently built binaries are available for:

### Android

Download [[here](https://drive.google.com/file/d/0BzcKPEavdc2NdUlfNTdmeTUwLTQ/edit?usp=sharing)].

This package in known to work on a number of popular modern devices. It is primarily developed on HTC One (M7).

### Windows

Search for *artifacts* [[here](https://ci.appveyor.com/project/johnmcfarlane/crag)].

This archive contains a copy of the demo and its necessary assets developed against Windows 7. All files must be expanded and the appropriate C run-time libraries installed. Your best bet is to install [this Microsoft package](https://www.microsoft.com/en-us/download/details.aspx?id=40784) first.

## Clone

The project is currently hosted on [GitHub](https://github.com/johnmcfarlane/crag).

```
git clone https://github.com/johnmcfarlane/crag.git
```

## Build/Run

Instructions are available for the following platforms:

- [GNU/Linux](README-linux.md) on PC and Raspberry Pi
- [Windows](README-windows.md) using MSVC++
- [Android](android/README.md)

## Licencing

Copyright 2009 - 2015 [John McFarlane](http://john.mcfarlane.name/). All rights reserved.

This software is distributed under the terms of the GNU General Public License.
