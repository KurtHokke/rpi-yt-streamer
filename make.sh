#!/usr/bin/bash

SOURCE_DIR="$(dirname -- "${BASH_SOURCE[0]}")"
build() {
    cd "$SOURCE_DIR" || return
    [ -d "${SOURCE_DIR}/build" ] && rm -rf "${SOURCE_DIR}/build"
    mkdir "${SOURCE_DIR}/build" && cd "${SOURCE_DIR}/build" && \
    cmake "$@" ..; CMAKE_EXITCODE=$?
    if [ "$CMAKE_EXITCODE" -ne 0 ]; then
        cd ..
    else
        make && make install && cd ..
    fi
    
}
server() {
    cd "$SOURCE_DIR" || return
    [ -f "./bin/server" ] && ./bin/server "$@"
}