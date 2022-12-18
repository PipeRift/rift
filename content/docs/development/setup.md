---
title: "Setup"
description: "Setting up Rift's development environment."
lead: "Setting up Rift's development environment."
draft: false
images: []
menu:
  docs:
    parent: "development"
weight: 310
toc: true
---

## Requirements

- A Cpp20 compiler (e.g: Clang 14 or higher)
- CMake

## Cloning Rift

First of all, you will need to clone the project from [{{<projectRepo>}}]({{<projectRepo>}}).

There are many ways to do this, but from the terminal you can for example run:

`git clone {{<projectRepo>}}`

### Initialize submodules
It is essential we also initialize all submodules (Rift's dependencies).

To do so, run the following command on the project's root folder:

`git submodule update --init --recursive`

### Install Dependencies (Linux)

Run the following to install needed dependencies on Linux.

Clang and CMake:
`sudo apt install clang cmake`
Other compilers with C++20 support can also be used.
  
Build essentials & graphics:
`sudo apt install build-essential xz-utils curl xorg-dev libx11-dev xlibmesa-glu-dev`


### Build Rift's LLVM

Rift uses a custom build process for LLVM which comes packed conveniently with the project, but requires manual setup.

This build can be easily triggered running the script `Programs/SetupLLVM.bat` (or `Programs/SetupLLVM.sh` on MacOS/Linux).

Sit and get comfortable, this might take a while.

Optionally, you could use a pre-built LLVM installation instead on MacOS or Linux. Not on Windows however, since its installation of LLVM does not contain certain elements that are required for us to build Rift.

## Configure & Build

Any CMake workflow you might like should work fine if you followed the previous steps.

If you for example were to use commands, you could first run `cmake -S . -B Build` to configure the project, and then `cmake --build Build` to build it.
