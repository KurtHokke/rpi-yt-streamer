#!/usr/bin/bash

SOURCE_DIR="$(dirname -- "${BASH_SOURCE[0]}")"
build() {
    cd "$SOURCE_DIR"; [ -d "./build" ] && rm -rf ./build; mkdir ./build && cd ./build && cmake "$@" .. && make && make install && cd ..
}
server() {
    cd "$SOURCE_DIR"; [ -f "./bin/server" ] && ./bin/server "$@"
}