# Instructions on how to build Crag on Android devices

Note: An Android package for ARM devices can be found [here](https://drive.google.com/file/d/0BzcKPEavdc2NdUlfNTdmeTUwLTQ/edit?usp=sharing).

## Introduction

This page provides information on how to build Crag for Android devices. It is written against Debian GNU/Linux 7.4 or Ubuntu 14.04 LTS.

### System Setup

The Crag project comes with an Android `setup` script which will attempt to prepare your Debian system to build and run Android builds. Be warned that this script - if successful - will do unforgivable things to your computer such as installing Oracle's Java Development Kit and adding 32-bit support to your OS.

1. `sudo apt-get install git`
2. `git clone https://github.com/johnmcfarlane/crag.git`
3. Download the following files to the crag/android/dl folder:
   * [Android SDK Tools](http://developer.android.com/sdk/index.html)
   * [Android NDK](http://developer.android.com/ndk/downloads/index.html)
4. Instal Oracle's JDK:
   * On Debian, Oracle's Java development kit is not available as a package and must be prepared as follows:
     * Download the [Java JDK](http://www.oracle.com/technetwork/java/javase/downloads/index.html) to the crag/android/dl folder.
     * From the crag/android folder, run:

       `script/setup_java`
   * On Ubuntu:

     ```
     sudo apt-get install python-software-properties
     sudo add-apt-repository ppa:webupd8team/java
     sudo apt-get update
     sudo apt-get install oracle-java7-installer
     ```
5. `crag/android/setup`
   * follow the instructions and enter your password when prompted
   * when the Android SDL Manager launches, select the following packages:
     * Tools:
       * Android SDK Platform-tools
       * Android SDK Build-tools
     * Android 4.0 (API 14)
       * SDK Platform
     * Extras
       * Android Support Library

### System Setup

From the android folder, type `./go debug N` where `N` is the number of CPUs on your PC. 
Remember to connect your phone to your PC!

### Debugging

To switch project to debug build:

* in crag/android/jni/Application.mk, switch APP_OPTIM from release to debug
* in crag/android/AndroidManifest.xml, add the android:debuggable attribute to the application tag
