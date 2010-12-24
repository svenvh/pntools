#!/bin/sh

if [ ! -f "local.am" ] ; then
  echo "# local.am  - Your own local automake additions" > local.am
fi

libtoolize -c
aclocal -I m4
#autoheader
automake -a -c --foreign
autoconf

