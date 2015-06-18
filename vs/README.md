# Instructions on how to build Crag for Windows.

## Introduction

This page provides information about building Crag under Windows. If you follow the instructions below, you should not need to deal with any computer code. However, a familiarity with getting C/C++ projects to build under Windows will be a major advantageous. Please feel free to provide feedback using the comments section at the bottom of this page. 

These instructions are all written with a view to getting a 32-bit build of crag to run on 32-bit Windows Vista and above. Crag requires C++11 features which only became available in VS2012 and are likely unsupported in XP and below. However, Crag is also intended to run on the subset of features available in GCC 4.7 which is available in [MinGW](www.mingw.org) so you may wish to consider that avenue if you're especially brave.

## 1 Prerequisites

Here are some pieces of software you will likely need.

### 1.1 Windows

Crag is built against Windows 7. Newer and older versions may work. The main limitation is likely to be the choice of compiler.

### 1.2 Visual C++ Express Edition 2013

This free version of Microsoft's IDE can be downloaded [here](http://www.microsoft.com/visualstudio/), although you may need to register to get an activation key.

### 1.3 DirectX SDK

This is required by SDL. Apparently, it's possible to flip some switches in SDL and avoid installing DirectX SDK but I haven't tried. You may find the correct download [here](http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=6812).

### 1.4 Git for Windows

Studio Express 2013 comes with Git integration but I am unable to make it work. Try [Git](http://git-scm.com/download/win) itself.

Studio 2015 RC has imporved Git integration. 
You can clone the Crag repository from inside the IDE and it give you the option of installing the above Git tools automatically.

## 2 Getting Crag Source Code

First, you need to get the crag source code from [GitHub](https://github.com/johnmcfarlane/crag).

## 3 Dependencies

Crag relies on a number of middleware libraries. The Crag project expect them to be locate in specific folders under the `vc` folder. They can all be built automatically using the crag solution. But each requires specific instructions to download and prepare.

### 3.1 SDL

The [Simple Direct-media Layer](http://www.libsdl.org/) provides everything a game needs to run on a variety of systems and hardware.

  * Download `SDL2-devel-2.0.X-VC.zip` from [here](http://www.libsdl.org/download-2.0.php).
  * Open the zip file and copy the contents of the `SDL-2.0.0-XXXX' folder to `crag\vc\SDL2'.

### 3.2 SDL_image

  * Download `SDL2_image-devel-2.0.X-VC.zip` [here](https://www.libsdl.org/projects/SDL_image/).
  * Open the zip file and copy the contents of the `SDL2_image-<blah>' folder to `crag\vs\SDL2_image'.

### 3.3 ODE

The [Open Dynamic Engine](http://www.ode.org/) is an open-source library which simulates rigid-body physics.

  * Follow the download link from the [ODE download page](https://bitbucket.org/odedevs/ode/downloads) and download the latest version (currently ode-0.13.1.tar.gz) into the crag/vs folder.
  * Open a Git Bash console and go to the crag/vs folder:
  
    ```
	cd crag/vs
	gunzip.exe ode-0.13.1.tar.gz
	tar -xf ode-0.13.1.tar
	mv ode-0.13.1 ode
	```

  * The ODE project files need to be built by running `vs\premake_ode.bat`. (More details of this step can be found [here](http://opende.sourceforge.net/wiki/index.php/Manual_(Install_and_Use)).
  * Open dependencies\ode\build\vs2008
  * Build DebugSingleLib and/or ReleaseSingleLib configurations
  * Add the dNODEBUG pre-processor definition to the DebugDoubleLib and ReleaseDoubleLib configurations. (Still needed?)

### 3.3 GLEW

The [GL Extension Wrangler](http://glew.sourceforge.net/) enables crag to use modern (21st century) OpenGL extensions. 

  * Download the GLEW binary from [here](http://glew.sourceforge.net/)
  * unzip it into crag\vs\glew

## 4 Building Crag

Finally, it's time to build the Crag binary. 

  * Launch Visual C++;
  * Open solution, `crag\vs\vs2013\VS2013.sln`;
  * From the standard toolbar, select the _Release_ solution configuration (or _Debug_ if you wish to debug crag);
  * From the Build menu, select _Build Solution_;
  * Wait for the build process to finish and ensure that there were no failures;
  * The complete Crag build can be located in vs\vs2008\Release\crag;

### 4.1 Debugging Crag from Studio

Don't forget to set the debugging: crag project properties, Debugging category, Working Directory to Debug or Release\crag

## 5 Troubleshooting

These instructions are completely untested by anyone other then myself. Please send me an email [here](http://www.google.com/profiles/110202519902799314719) if you hit any problems and I'll try and compile a list of the most obvious pitfalls and their resolutions right here.

Good luck!
