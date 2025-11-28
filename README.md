# Charging-pile-gateway
This is a gateway module for a smart charging pile system.
gateway/
├── CMakeLists.txt
├── cmake/
│   └── arm_toolchain.cmake
├── scripts/
│   ├── toolchain_env.sh
│   ├── gen_ramdisk.sh
│   ├── make_flash_image.sh
│   └── tftp_nfs_setup.sh
├── include/
│   ├── common/
│   │   └── config.h
│   ├── log.h
│   ├── thread_pool.h
│   ├── io_service.h
│   ├── ipc.h
│   └── sqlite_wrapper.h
├── src/
│   ├── CMakeLists.txt
│   ├── main.c
│   ├── log/
│   │   └── log.c
│   ├── core/
│   │   ├── thread_pool.c
│   │   └── io_service.c
│   ├── ipc/
│   │   └── unixsock.c
│   ├── storage/
│   │   └── sqlite_wrapper.c
│   └── proto/
│       └── messages.proto
└── docs/
    └── partition_layout.md
