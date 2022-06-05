@echo off

pushd "%~dp0../"
git clone --depth 1 --branch v14.0.5 https://github.com/PipeRift/rift-llvm.git Extern/rift-llvm
cd Extern/rift-llvm
git pull origin main
git submodule update --init --recursive
python build.py --config Debug
python build.py --config Release
python build.py --config RelWithDebInfo
python build.py --config MinSizeRel
popd
