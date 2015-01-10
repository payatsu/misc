#!/bin/sh

# @file This is a program for downloading, building, installing libbpg.

# install libpng 1.6 or later, at first.

mkdir /usr/local/src/libpng
cd /usr/local/src/libpng

wget ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.16.tar.gz
tar xzvf libpng-1.6.16.tar.gz

cd libpng-1.6.16
./configure && make check && make install

#

mkdir /usr/local/src/libbpg
cd /usr/local/src/libbpg

wget http://bellard.org/bpg/libbpg-0.9.4.tar.gz
tar xzvf libbpg-0.9.4.tar.gz

cd libbpg-0.9.4
make -j4 && make install
