#!/bin/sh
if test -n "$1"
then
    git archive --format tar --remote=$1 --output=meegotouch-applauncherd-0.12.1.tar --prefix meegotouch-applauncherd-0.12.1/ released/0.12.1
    bzip2 meegotouch-applauncherd-0.12.1.tar
else
    echo "Usage: create-tarball.sh <repo>"
    echo "       where <repo> is suitable for git archive --remote=<repo>"
fi
