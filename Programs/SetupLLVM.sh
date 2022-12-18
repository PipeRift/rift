#!/bin/sh
cd ..
git clone --depth 1 --branch v15.x https://github.com/PipeRift/rift-llvm.git Extern/rift-llvm
cd Extern/rift-llvm
git pull origin main
python init.py
python build.py --config Debug
python build.py --config Release
