#!/usr/bin/bash

! command -v git >/dev/null 2>&1 && echo "Error: Git not available" >&2 && exit 11
! command -v curl >/dev/null 2>&1 && echo "Error: curl not available" >&2 && exit 12

git submodule update --init --recursive

ARCH=$(uname -m)

USE_PYPY="$1"
INSTALL_PREFIX="$2"
VENV="${INSTALL_PREFIX}/bin/_venv"
PATH_TO_3RD_DIR="$3"
DL_PATH="${INSTALL_PREFIX}/dl_path"

ensure_pypyvenvpip() {
    [[ -f "${VENV}/.venv_installed" ]] && return 0
    if ! command -v pypy3 >/dev/null 2>&1; then
        echo "Error: Pypy3 not available" >&2
        echo "On Debian/Ubuntu, try: sudo apt-get install pypy3" >&2
        echo "On Arch-Linux, try: sudo pacman -S pypy3" >&2
        return 1
    fi
    tmp="$(mktemp -d)"
    if ! curl -L -o "${tmp}/virtualenv.pyz" "https://bootstrap.pypa.io/virtualenv.pyz"; then
        rm -rf "${tmp}"
        echo "Error: failed to download virtualenv.pyz" >&2
        return 3
    fi
    if ! pypy3 "${tmp}/virtualenv.pyz" --python="$(which pypy3)" "${VENV}"; then
        rm -rf "${tmp}"
        echo "Error: failed to create virtual env" >&2
        return 4
    fi
    rm -rf "${tmp}"
    touch "${VENV}/.venv_installed"
    return 0
}
get_python_v() {
    python_version=$(python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
    return "$python_version"
}
ensure_pyvenvpip() {
    if ! command -v python3 >/dev/null 2>&1; then
        echo "Error: Python3 not available" >&2
        return 1
    fi
    if ! python3 -c "import sys; sys.exit(0 if sys.version_info >= (3, 9) else 1)"; then
        echo "Error: Python $(get_python_v) is too old - requires 3.9+ for venv" >&2
        return 2
    fi
    if ! python3 -c "import venv" 2>/dev/null; then
        echo "Error: Python venv module not available" >&2
        echo "On Debian/Ubuntu, try: sudo apt-get install python3-venv" >&2
        return 3
    fi
    if ! command -v pip3 >/dev/null 2>&1; then
        echo "Error: Pip3 not available" >&2
        echo "On Debian/Ubuntu, try: sudo apt-get install python3-pip" >&2
        echo "On Arch-Linux, try: sudo pacman -S python-pip" >&2
        return 4
    fi
    return 0
}
setup_venv() {
    [[ -f "${VENV}/bin/activate" ]] && return 0
    if ! python3 -m venv "${VENV}" >/dev/null 2>&1; then
        echo "Error: Failed to create venv" >&2
        return 1
    fi
    if [[ ! -f "${VENV}/bin/activate" ]]; then
        echo "Error: Failed to find ${VENV}/bin/activate" >&2
        return 2
    fi
    return 0
}
ensure_deps() {
    [[ -f "${VENV}/.deps_installed" ]] && return 0
    if [[ ! -f "${PATH_TO_3RD_DIR}/yt-dlp/devscripts/install_deps.py" ]]; then
        echo "Error: ${PATH_TO_3RD_DIR}/yt-dlp/devscripts/install_deps.py does not exist" >&2
        return 1
    fi
    if [[ ! -f "${PATH_TO_3RD_DIR}/yt-dlp/devscripts/make_lazy_extractors.py" ]]; then
        echo "Error: ${PATH_TO_3RD_DIR}/yt-dlp/devscripts/make_lazy_extractors.py does not exist" >&2
        return 2
    fi
    [[ -f "${VENV}/bin/activate" ]] && source "${VENV}/bin/activate" || return 3
    #which python3
    #which pip3
    (cd "${PATH_TO_3RD_DIR}/yt-dlp" || return
    python3 "${PATH_TO_3RD_DIR}/yt-dlp/devscripts/install_deps.py" && touch "${VENV}/.deps_installed" && \
    python3 "${PATH_TO_3RD_DIR}/yt-dlp/devscripts/make_lazy_extractors.py")
}
ensure_ffmpeg() { local tarball_url; local filename; local tmp
    command -v "${INSTALL_PREFIX}/bin/ffmpeg" >/dev/null 2>&1 && command -v "${INSTALL_PREFIX}/bin/ffprobe" >/dev/null 2>&1 && return 0
    case "$ARCH" in
        "x86_64")
            tarball_url="https://github.com/yt-dlp/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-linux64-gpl.tar.xz"
        ;;
        "aarch64")
            tarball_url="https://github.com/yt-dlp/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-linuxarm64-gpl.tar.xz"
        ;;
        *)
    esac
    filename="${tarball_url##*/}"
    tmp="$(mktemp -d)"
    if ! curl -L -o "${tmp}/$filename" "$tarball_url"; then
        rm -rf "$tmp"
        echo "Error: failed to curl ffmpeg" >&2
        return 1
    fi
    if ( cd "$tmp"; ! tar -xvf "$filename" && return 2
         ! mv ./*/bin/ff* "${INSTALL_PREFIX}/bin/" && return 3 ); then
        local ret=$?
        rm -rf "$tmp"
        echo "Error: failed to move ffmpeg bins" >&2
        return $ret
    fi
    rm -rf "$tmp"
    return 0
}
copy_ytdlp() {
    [[ -d "${INSTALL_PREFIX}/bin/yt-dlp/yt_dlp" ]] && return 0
    if ! mkdir -p "${INSTALL_PREFIX}/bin/yt-dlp"; then
        echo "Error: failed to mkdir -p \"${INSTALL_PREFIX}/bin/yt-dlp\"" >&2
        return 1
    fi
    if ! cp -r "${PATH_TO_3RD_DIR}/yt-dlp/yt_dlp" "${INSTALL_PREFIX}/bin/yt-dlp/yt_dlp"; then
        echo "Error: failed to cp -r \"${PATH_TO_3RD_DIR}/yt-dlp/yt_dlp\" \"${INSTALL_PREFIX}/yt_dlp\""
        return 2
    fi
    return 0
}

create_servicefile() {
[ ! -d "./service" ] && mkdir -p ./service
cat <<EOF > ./service/yt-dlp-server.service
[Unit]
Description=yt-dlp-server for raspberry pi
After=network.target

[Service]
ExecStart=${INSTALL_PREFIX}/bin/yt-dlp-server
WorkingDirectory=${INSTALL_PREFIX}/bin
Restart=always
User=${USER}
Group=${USER}

[Install]
WantedBy=multi-user.target
EOF
}

[[ ! -d "$DL_PATH" ]] && mkdir -p "$DL_PATH"

if [[ "$USE_PYPY" == "true" ]]; then
    if ! ensure_pypyvenvpip; then
        exit $?
    fi
else
    if ! ensure_pyvenvpip; then
        exit $?
    fi
    if ! setup_venv; then
        exit $?
    fi
fi
if ! ensure_deps; then
    exit $?
fi
if ! ensure_ffmpeg; then
    exit $?
fi
if ! copy_ytdlp; then
    exit $?
fi
create_servicefile
exit 0