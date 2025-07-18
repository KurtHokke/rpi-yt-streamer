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
    [ -f "./bin/yt-dlp-server" ] && ./bin/yt-dlp-server "$@"
}
cli_to_api() {
    ( source "${SOURCE_DIR}/bin/_venv/bin/activate" && \
      python "${SOURCE_DIR}/3rd/yt-dlp/devscripts/cli_to_api.py" "$@" )
}