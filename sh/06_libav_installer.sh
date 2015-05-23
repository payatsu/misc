#!/bin/sh

# @file This is a program for downloading, building, installing libav.

# Libavのインストール前に、LICENCEファイルを熟読せよ

apt-get install --yes yasm

apt-get install --yes libmp3lame-dev libvpx-dev libwebp-dev libx264-dev libxext-dev libxfixes-dev

mkdir -p /usr/local/src/libav
cd /usr/local/src/libav

wget https://libav.org/releases/libav-11.1.tar.gz
tar xzvf libav-11.1.tar.gz

mkdir -p /usr/local/src/libav/build
cd /usr/local/src/libav/build

/usr/local/src/libav/libav-11.1/configure --enable-gpl \
		--enable-libmp3lame --enable-libvpx --enable-libwebp --enable-libx264 --enable-x11grab

# libfaac -> libfaac is nonfree and --enable-nonfree is not specified.
# libx265 -> x265 not found

make -j4

make install
