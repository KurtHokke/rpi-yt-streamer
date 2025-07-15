#!/usr/bin/bash

VENV="$1"
#echo "${@[1]}" > out.txt
#echo "${*[1]}"
git show
exit 0

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
    if ! python3 -m venv "${VENV}" >/dev/null 2>&1; then
        echo "Error: Failed to create venv" >&2
        return 1
    fi
    if [[ ! -f "${VENV}/bin/activate" ]]; then
        echo "Error: Failed to find ${VENV}/bin/activate" >&2
        return 2
    fi
    source "${VENV}/bin/activate" && which python3 && return 0 || return 3
}
if ! ensure_pyvenvpip; then
    exit $?
fi
if ! setup_venv; then
    exit $?
fi
exit 0