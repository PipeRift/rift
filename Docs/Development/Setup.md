# Setup

## Windows

### 1. Install llvm from source

Clone repository [`https://github.com/llvm/llvm-project`](https://github.com/llvm/llvm-project) into a folder of your choice. For rift, it is recommended you install 13.0 or higher.
`git clone https://github.com/llvm/llvm-project.git --branch release/13.x --depth 1`

NOTE: LLVM needs to be installed from source because binary installations dont provide cmake support in windows.

Now, build LLVM following the [instructions](https://github.com/llvm/llvm-project#getting-the-source-code-and-building-llvm).<br>
Some tested steps are:
- Generate with `cmake -S llvm -B build -DCMAKE_BUILD_TYPE=Release`
- Build with `cmake --build build`

### 2. Clone Rift locally
Clone Rift from [`https://github.com/PipeRift/rift`](https://github.com/PipeRift/rift) into a folder of your choice.

Make sure all submodules have been initialized. You can run `Programs/InitSubmodules.bat` to do it (or `git submodule update --init --recursive`).

### 2. Use an IDE
Any IDE supporting CMake is suitable.

Before we start building the project, we need to tell it where our LLVM installation is located.
We do this by setting `RIFT_LLVM_PATH` variable in cmake cache to our llvm path.


## Linux

### 1. Install dependencies
Make sure your system is updated with `sudo apt update` and `sudo apt upgrade` first.

Install LLVM and Clang 13
`sudo apt install clang-13`

Install dependencies
`sudo apt install build-essential xz-utils curl xorg-dev libx11-dev xlibmesa-glu-dev`

### 2. Clone Rift locally
Clone Rift from [`https://github.com/PipeRift/rift`](https://github.com/PipeRift/rift) into a folder of your choice.

Make sure all submodules have been initialized (with `git submodule update --init --recursive`).

### 2. Use an IDE
Any IDE supporting CMake is suitable.

NOTE: In Linux we don't need to specify the path to LLVM since it will be detected, but if we have multiple installations, we can choose which one by setting cmake variable `RIFT_LLVM_PATH` to the path we want.
