#!/bin/sh
# Run pkg-config with PKG_CONFIG_PATH set to the source tree.
env PKG_CONFIG_PATH=../../../../data/pkgconfig pkg-config $*
