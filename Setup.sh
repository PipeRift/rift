#!/bin/bash
apt-get -y update
apt-get -y upgrade

apt-get -y install build-essential xz-utils curl
apt-get -y install clang clang-tidy clang-format

apt-get -y install cmake

apt-get -y install python3 python3-pip
pip3 install conan

ln -s ./Build/compile_commands.json ./compile_commands.json