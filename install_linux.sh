#/bin/bash

LINUX_DIR=third_party/openFrameworks/scripts/linux
[[ -d $LINUX_DIR ]] || {
    printf "Directory '%s' does not exist, did you clone openFrameworks?\n" >&2
    exit 1
}

DISTROS=(`cd "$LINUX_DIR"; ls -d */ | sed -nr '/^extra/n;s|(.*)/$|\1|p'`)

[[ " ${DISTROS[*]} " =~ " $1 " ]] || {
    printf "Provide a Linux distribution name to install dependencies, one of:\n%s\n" "${DISTROS[*]}" >&2
    exit 1
}

DISTRO_DIR="$LINUX_DIR"/$1

function run {
    local cmd="$1"

    printf "<< Running '%s' ... >>\n" "$cmd"
    bash "$cmd"
    local ret=$?
    (( $ret == 0 )) && return 0

    printf "\nCommand '%s' failed!\n" "$cmd" >&2
    exit $ret
}

run "$DISTRO_DIR"/install_codecs.sh
run "$DISTRO_DIR"/install_dependencies.sh
run "$LINUX_DIR"/download_libs.sh

printf '\nSuccess.\nNow you can run `make`.\n'
exit 0
