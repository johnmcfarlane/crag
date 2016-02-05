# Instructions on how to build Crag for Windows.

## Introduction

This page provides information about building Crag under Windows. 
If you follow the instructions below, you should not need to deal with any computer code. 
However, a familiarity with getting C/C++ projects to build under Windows will be a major advantageous. 
Please feel free to provide feedback. 

These instructions are all written with a view to getting a 32-bit build of crag to run on 32-bit Windows 7 and above. 
Crag requires C++11 features which only became available in VS2015 and are likely unsupported in XP and below. 
However, Crag is also intended to run on the subset of features available in GCC 4.8 which is available in [MinGW](www.mingw.org) so you may wish to consider that avenue if you're especially brave.

## 1 Prerequisites

Here are some pieces of software you will likely need.

### 1.1 Windows

Crag is built against Windows 7. Newer and older versions may work. The main limitation is likely to be the choice of compiler.

### 1.2 Visual C++ Express Edition 2015

This free version of Microsoft's IDE can be downloaded [here](http://www.microsoft.com/visualstudio/), although you may need to register to get an activation key.

### 1.3 DirectX SDK

This is required by SDL. Apparently, it's possible to flip some switches in SDL and avoid installing DirectX SDK but I haven't tried. 
You may find the correct download [here](http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=6812).

## 2 Getting Crag Source Code

First, you need to get the crag source code from [GitHub](https://github.com/johnmcfarlane/crag).
VS2015 comes with Git support as well as a GitHub plugin.
There are instructions on how to clone the repo in [the main README.md](../README.md).

## 3 Dependencies

Crag relies on a number of middleware libraries. 
The Crag project expects them to be locate in specific folders under the `vc` folder. 

### 3.1 SDL

The [Simple Direct-media Layer](http://www.libsdl.org/) provides everything a game needs to run on a variety of systems and hardware.

1. Download `SDL2-devel-2.0.X-VC.zip` from [here](http://www.libsdl.org/download-2.0.php).
2. Open the zip file and copy the contents of the `SDL-2.0.X-XXXX' folder to `crag\vs\SDL2'.

Note: The current build of SDL2 does not get on well with VS2015. You may need to follow [this advice](http://stackoverflow.com/questions/28247992/how-to-fix-unresolved-externals-of-sdl-2-0-3-on-visual-studio-2015-preview#comment50936657_28247992).

### 3.2 SDL_image

1. Download *SDL2_image-devel-2.0.X-VC.zip* from the  [SDL_image website](https://www.libsdl.org/projects/SDL_image/).
2. Open the zip file and copy the contents of the *SDL2_image-2.0.X* folder to *crag\vs\SDL2_image*.

### 3.3 ODE

The [Open Dynamic Engine](http://www.ode.org/) is an open-source library which simulates rigid-body physics.
The ODE project is included in the crag solution but that project needs some preparation:

1. Follow the download link from the [ODE download page](https://bitbucket.org/odedevs/ode/downloads) and download the latest version (currently ode-0.13.tar.bz2) into the crag/vs folder.
2. Open a Git Bash console and go to the crag/vs folder:

   ```
   cd crag/vs
   gunzip2.exe ode-0.14.tar.bz2
   tar -xf ode-0.14.tar
   mv ode-0.14 ode
   ```

3. The ODE project files need to be built by running *vs\crag\premake_ode.bat*. (More details of this step can be found [here](http://opende.sourceforge.net/wiki/index.php/Manual_(Install_and_Use)).
4. Open *crag\vs\ode\build\vs2008\ode.sln* and click OK to the 'One way upgrade'.
5. Build *DebugSingleLib* and/or *ReleaseSingleLib* configurations

### 3.4 GLEW

The [GL Extension Wrangler](http://glew.sourceforge.net/) enables crag to use modern OpenGL extensions. 

1. Download the GLEW binary from [the GLEW website](http://glew.sourceforge.net/).
2. Unzip it into *crag\vs\glew*.

## 4 Building Crag

Finally, it's time to build the Crag binary. 

1. Launch Visual C++;
2. Open solution, `crag\vs\crag\crag.sln`;
3. From the standard toolbar, select the _Release_ solution configuration (or _Debug_ if you wish to debug crag);
4. From the Build menu, select _Build Solution_;
5. Wait for the build process to finish and ensure that there were no failures;
6. The Crag binary can be located in `crag\vs\crag\Release\crag.exe`.

## 5 Running Crag

You can run Crag from within Visual Studio from the Debug -> Start Debugging menu option.
The working directory is the repository root and a couple of DLLs must be located there:

* SDL.dll
* SDL_image.dll

## 5 Troubleshooting

These instructions are completely untested by anyone other then myself. 
Please send me an email [here](http://www.google.com/profiles/110202519902799314719) if you hit any problems and I'll try and compile a list of the most obvious pitfalls and their resolutions right here.

Good luck!

### 5.1 Crash in Release build

There is a known bug when switching from running Debug build to running Release builds. You might try deleting crag.cfg from the `crag` root folder.
