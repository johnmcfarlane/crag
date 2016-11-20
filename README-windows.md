# Instructions on how to build Crag for Windows.

[![Build status](https://ci.appveyor.com/api/projects/status/b1x763k4ydpiwyuu/branch/develop?svg=true)](https://ci.appveyor.com/project/johnmcfarlane/crag/branch/develop)

## Introduction

This page provides information about building Crag under Windows. 
If you follow the instructions below, you should not need to deal with any computer code. 
However, a familiarity with getting C/C++ projects to build under Windows will be a major advantageous. 
Please feel free to provide feedback. 

These instructions are all written with a view to getting a 64-bit build of crag to run on 64-bit Windows 7 and above. 
Crag requires C++11 features which only became available in VS2015 and are likely unsupported in XP and below. 
However, Crag is also intended to run on the subset of features available in GCC 4.8 which is available in [MinGW](www.mingw.org) so you may wish to consider that avenue if you're especially brave.

## 1 Prerequisites

Here are some pieces of software you will likely need:

* [CMake](https://cmake.org/download/) (Select "Add CMake to the system PATH")
* [Git](https://git-scm.com/download/win) (also integrated with [Visual Studio](https://git-scm.com/book/en/v2/Git-in-Other-Environments-Git-in-Visual-Studio))
* [Microsoft Visual Studio Community 2015](http://www.microsoft.com/visualstudio/) (Install C++ )
* Windows 7 (or newer)

## 2 Building Crag

Execute the following from the Windows Command Prompt (`cmd`):

1. Clone the project from [Github](https://github.com/johnmcfarlane/crag/):

   ```shell
   git clone git@github:johnmcfarlane/crag
   ```

2. Add vsvars32.bat to your path:

   ```shell
   "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"
   ```

3. Build and run a 32-bit, debug binary in your chosen build directory:

   ```shell
   cmake .
   MSBuild.exe crag.sln
   cd src
   crag.exe
   ```

   or a 64-bit Release binary:
   
   64-bit Release
   ```shell
   cmake -G "Visual Studio 14 2015 Win64" .
   MSBuild.exe crag.sln /p:Platform=x64 /p:Configuration=Release
   cd src
   crag.exe
   ```

4. Install:

   ```
   cmake -DCMAKE_INSTALL_PREFIX:PATH=some_place .
   MSBuild.exe crag.sln
   MSBuild.exe INSTALL.vcxproj
   ```
