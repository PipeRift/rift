import os
import subprocess
import sys
import argparse


def main(argv):
  rift_llvm_version = 'v15.0.6'

  this_file_path = os.path.dirname(__file__)
  rift_path = os.path.dirname(this_file_path)

  parser = argparse.ArgumentParser(description = "Setup Rift's LLVM toolchain", formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  args = parser.parse_args()

  rift_llvm_relpath = 'Extern/rift-llvm'
  rift_llvm_path = os.path.join(rift_path, rift_llvm_relpath)

  if not os.path.exists(rift_llvm_path):
    print('> Downloading rift-llvm')
    subprocess.run('git clone -s --depth 1 --branch {} https://github.com/PipeRift/rift-llvm.git {}'.format(rift_llvm_version, rift_llvm_path), check=True)
    os.chdir(rift_llvm_path)
  else:
    print('> Updating rift-llvm')
    os.chdir(rift_llvm_path)
    os.system('git pull origin {}'.format(rift_llvm_version))

  print('\n> Init LLVM')
  os.system('python init.py')

  print('\n> Build LLVM')
  os.system('python build.py --config Debug')
  os.system('python build.py --config Release')
  os.system('python build.py --config RelWithDebInfo')
  os.system('python build.py --config MinSizeRel')

if __name__ == "__main__":
  main(sys.argv[1:])

