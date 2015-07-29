#!/bin/sh

# @file This is a program for downloading, building, installing libbpg.

# install libpng 1.6 or later, at first.

SUDO='sudo '
LIBPNG_VERSION=libpng-1.6.16
LIBBPG_VERSION=libbpg-0.9.4

${SUDO}mkdir -p /usr/local/src/libpng
cd /usr/local/src/libpng

if [ ! -d ${LIBPNG_VERSION} ] ;
then
	if ! wget -O- ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/${LIBPNG_VERSION}.tar.gz | ${SUDO}tar xzf ;
	then
		echo \`wget\' failed. 1>&2
		exit 1
	fi
fi

cd ${LIBPNG_VERSION}
if [ ! -f Makefile ] ;
then
	if ! ${SUDO}./configure ;
	then
		echo \`./configure\' failed. 1>&2
		exit 1
	fi
fi

if ! ${SUDO}make check ;
then
	echo \`make\' failed. 1>&2
	exit 1
fi

if ! ${SUDO}make install ;
then
	echo \`make install\' failed. 1>&2
	exit 1
fi

#

${SUDO}mkdir -p /usr/local/src/libbpg
cd /usr/local/src/libbpg

if [ ! -d ${LIBBPG_VERSION} ] ;
then
	if ! wget -O- http://bellard.org/bpg/${LIBBPG_VERSION}.tar.gz | ${SUDO}tar xzf ;
	then
		echo \`wget\' failed. 1>&2
		exit 1
	fi
fi

cd ${LIBBPG_VERSION}

if ! make ;
then
	echo \`make\' failed. 1>&2
	exit 1
fi

if ! make install ;
then
	echo \`make install\' failed. 1>&2
	exit 1
fi

exit 0
