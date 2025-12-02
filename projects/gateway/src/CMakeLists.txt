cmake_minimum_required(VERSION 3.13)
project(gateway_app C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -g -fPIC")

include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/proto)

file(GLOB SRC_FILES
    main.c
    core/*.c
    log/*.c
    ipc/*.c
    storage/*.c
    proto/*.c
)

list(APPEND SRC_FILES
    net/mqtt_client.c
    web/http_server.c
    ../thirdparty/civetweb.c 
)

# find libraries (adjust or set manually if cross-compiling)
find_library(LIBRT rt)
find_package(Threads REQUIRED)

add_executable(gatewayd ${SRC_FILES})
target_link_libraries(gatewayd
    ${LIBRT} ${CMAKE_THREAD_LIBS_INIT}
    -lpthread -ldl -lm -lsqlite3 -lprotobuf-c -lmosquitto
)

install(TARGETS gatewayd DESTINATION bin)
