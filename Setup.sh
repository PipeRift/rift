#!/bin/bash
sudo apt -y update
sudo apt -y upgrade

sudo apt -y install build-essential xz-utils curl
sudo apt -y install libx11-dev xorg-dev libglu1-mesa-devz
sudo apt -y install llvm clang clang-tidy clang-format

sudo apt -y install gdb

sudo apt -y install cmake
