#!/bin/bash
set -e
set -x
cd $(dirname "$0")

# To remove a module: git submodule deinit $ROOT/$DIR

function checkout() {
    DIR=$1
    URL=$2
    ROOT=.
    GIT_USER=$(sed -n '/email = /s/.*= \(.*\)@.*/\1/p' ~/.gitconfig)
    if [[ -z $GIT_USER ]]; then set +x; echo "Git user not found"; exit 1; fi

    if [[ ! -d $ROOT/$DIR ]]; then
      git submodule add $URL $ROOT/$DIR
    fi

    git submodule update --init $ROOT/$DIR
}

checkout DigiFi https://github.com/digistump/DigiFi.git

# checkout layout_wifi/microSRCP https://github.com/mc-b/microSRCP.git

# checkout layout_wifi/translate_go/src/golang.org/x/crypto https://go.googlesource.com/crypto

checkout layout_wifi/translate_go/src/github.com/stretchr/testify https://github.com/stretchr/testify.git
