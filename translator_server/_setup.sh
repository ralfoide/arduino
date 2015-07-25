#!/bin/bash
_BS=$(dirname "$BASH_SOURCE")
if [[ -d "$_BS" ]]; then
    export GOPATH=$(readlink -f $PWD/$_BS)
    echo "GOROOT=$GOROOT"
    echo "GOPATH=$GOPATH"
fi
