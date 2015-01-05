# Instructions on how to build Crag on Android devices

Note: An Android package for ARM devices can be found [here](https://drive.google.com/file/d/0BzcKPEavdc2NdUlfNTdmeTUwLTQ/edit?usp=sharing).

## Introduction

This page provides information on how to build Crag for Android devices. It is written against Debian GNU/Linux 7.4. Please feel free to provide feedback using the link at the bottom of this page.

### System Setup

The Crag project comes with an Android `setup` script which will attempt to prepare your Debian system to build and run Android builds. Be warned that this script - if successful - will do unforgivable things to your computer such as installing Oracle's Java Development Kit and adding 32-bit support to your OS.

1. `sudo apt-get install git`
1. follow [these instructions](https://code.google.com/p/crag/source/checkout) to clone the crag project
1. Download the following files to the crag/android/dl folder:
   * [Android SDK Tools](http://developer.android.com/sdk/index.html)
   * [Android NDK](http://developer.android.com/tools/sdk/ndk/index.html)
   * [Java JDK](http://www.oracle.com/technetwork/java/javase/downloads/index.html)
1. `crag/android/setup`
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

From the android folder, type `./go debug 4` where 4 is the number of CPUs. 

### Debugging

To switch project to debug build:

* in crag/android/jni/Application.mk, switch APP_OPTIM from release to debug
* in crag/android/AndroidManifest.xml, add the android:debuggable attribute to the application tag
