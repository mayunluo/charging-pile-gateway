#!/bin/bash
# usage: ./make_flash_image.sh uboot.bin zImage rootfs.img
set -e
if [ $# -ne 3 ]; then
  echo "Usage: $0 uboot.bin zImage rootfs.img"
  exit 1
fi

UBOOT=$1; KERNEL=$2; ROOTFS=$3
OUT=flash.img
# simplistic: append files; real boards require padding/offsets per layout
cp ${UBOOT} ${OUT}
dd if=${KERNEL} of=${OUT} bs=1K seek=1024 conv=notrunc
dd if=${ROOTFS} of=${OUT} bs=1K seek=8192 conv=notrunc
echo "Created flash image: ${OUT}"
