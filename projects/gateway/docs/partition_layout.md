# Flash partition layout (example)
Offset / Size (KB)
- 0x0000 - 0x1000 (4MB): u-boot
- 0x1000 - 0x2000 (4MB): u-boot env
- 0x2000 - 0x4000 (8MB): kernel (zImage)
- 0x4000 - 0x20000 (128MB): rootfs_A
- 0x20000 - 0x3C000 (128MB): rootfs_B
- rest: userdata (logs, downloaded images)
