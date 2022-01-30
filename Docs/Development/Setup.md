# Setup

## Windows

### 1. Install llvm from source

Clone repository [`https://github.com/llvm/llvm-project`](https://github.com/llvm/llvm-project) into a folder of your choice. For rift, it is recommended you install 13.0 or higher.
`git clone https://github.com/llvm/llvm-project.git --branch release/13.x --depth 1`

NOTE: LLVM needs to be installed from source because binary installations dont provide cmake support in windows.

Now, build LLVM following the [instructions](https://github.com/llvm/llvm-project#getting-the-source-code-and-building-llvm).<br>
Some tested steps are:
- Generate with `cmake -S llvm -B build`
- Build with `cmake --build build`

### 2. Clone Rift locally

Clone Rift from [`https://github.com/PipeRift/rift`](https://github.com/PipeRift/rift) into a folder of your choice.

Make sure all submodules have been initialized. You can run `Programs/InitSubmodules.bat` to do it.

### 2. Use an IDE

Any IDE supporting CMake is suitable.

Before we start building the project, we need to tell the project where our LLVM installation is located.


## Linux

### Install dependencies
TO BE TESTED

Make sure your system is updated with `sudo apt update` and `sudo apt upgrade`

Install LLVM and Clang 13
`sudo apt install clang-13`

Install Rift dependencies
`sudo apt install build-essential xz-utils curl xorg-dev libx11-dev xlibmesa-glu-dev`

