#!/bin/bash

set -euo pipefail

make clean
export MODE=asan
ci/build.sh

rm -rf linux-kernel
if [ -e ../linux-kernel ]; then
    git clone ../linux-kernel linux-kernel
else
    git clone --depth=3 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git linux-kernel
fi
cd linux-kernel
# Make most __CHECKER__ checks ineffective.
{ git grep -lw __CHECKER__ || [ $? -eq 1 ]; } |
    { grep -vE '^arch/|^include/linux/compiler|^include/uapi/linux/types.h|^include/linux/bug.h' || [ $? -eq 1 ]; } |
    xargs -r -d '\n' sed -i 's/__CHECKER__/___CHECKER___/g'
make defconfig
export ASAN_OPTIONS=detect_invalid_pointer_pairs=2
export UBSAN_OPTIONS=halt_on_error=1
make -sj"$(nproc)" C=1 CHECK="${PWD}/../sparse"
