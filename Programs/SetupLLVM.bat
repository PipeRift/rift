@echo off
cd ..
git clone --depth 1 --branch v14.0.5 https://github.com/PipeRift/rift-llvm.git Extern/rift-llvm
cd Extern/rift-llvm
git pull origin main
git submodule update --init --recursive
python build.py --config Debug
python build.py --config Release
