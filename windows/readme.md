# README - HOW TO BUILD AND RUN LUTECONV ON WINDOWS

* 2-Nov-2020
* Luke Emmet <luke.emmet [at] orlando-lutes [dot] com>

This folder contains a Visual Studio project and extra header and library files needed to build and run luteconv on MS windows

# Tested configuration

The following configuration and setup was used to build and test luteconv

* platform: Windows 10, x64
* IDE: Visual Studio 2017, community edition

# How to build

Currently only a 32bit x86 build option is configured.

* Open the luteconv.sln in Visual studio.
* Choose the Release and x86 build configuration from the build options
* Build the project

You may see some warnings about gmtime, these are harmless.

If all is successful, a sub folder "Release" will be created and luteconv.exe will be created in that folder

You will also need to copy the following dlls into any folder from which you want to run luteconv/exe:

* libs\win32\libz\libz.dll
* libs\win32\libzip\libzip.dll

