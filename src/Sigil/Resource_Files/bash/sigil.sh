#!/bin/sh

# Entry point for Sigil on Unix systems.
# Adds the directory the Sigil executable is located
# in to the LD_LIBRARY_PATH so the custom Qt libs
# are recognized and loaded.

appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH
$dirname/$appname $* 
