#!/bin/sh

export PATH=$PATH:/root/workspace/lichen/software/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/
make clean
make
cp ./tuart /home/user/nfsroot/rootfs/bin
