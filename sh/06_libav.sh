#!/bin/sh

# @file This is a program for downloading, building, installing libav.
# Libavのインストール前に、LICENCEファイルを熟読せよ

LIBAV_VERSION=libav-11.4
SUDO='sudo '

${SUDO}apt-get update
${SUDO}apt-get install --yes yasm
${SUDO}apt-get install --yes \
	   frei0r-plugins-dev \
	   libgnutls-dev \
	   libbs2b-dev \
	   libcdio-dev \
	   libcdio-paranoia-dev \
	   libdc1394-22-dev \
	   libfreetype6-dev \
	   libgsm1-dev \
	   libmp3lame-dev \
	   libopencore-amrnb-dev \
	   libopencore-amrwb-dev \
	   libopencv-dev \
	   libopenjpeg-dev \
	   libopus-dev \
	   libpulse-dev \
	   librtmp-dev \
	   libschroedinger-dev \
	   libspeex-dev \
	   libtheora-dev \
	   libtwolame-dev \
	   libvo-aacenc-dev \
	   libvo-amrwbenc-dev \
	   libvorbis-dev \
	   libvpx-dev \
	   libwavpack-dev \
	   libwebp-dev \
	   libx264-dev \
	   libxvidcore-dev \
	   libssl-dev \
	   libx11-dev \
	   libxext-dev libxfixes-dev
# 	   libx265-dev

SRC_DIR=/usr/local/src/libav/
${SUDO}mkdir -p ${SRC_DIR}

cd ${SRC_DIR}
if [ ! -d ${LIBAV_VERSION} ] ;
then
	if ! wget -O- https://libav.org/releases/${LIBAV_VERSION}.tar.gz | ${SUDO}tar xzf - ;
	then
		echo \`wget\' failed. 1>&2
		exit 1
	fi
fi

BUILD_DIR=./build-${LIBAV_VERSION}
${SUDO}mkdir -p ${BUILD_DIR}

cd ${BUILD_DIR}
if [ ! -f Makefile ] ;
then
	if ! ${SUDO}${SRC_DIR}${LIBAV_VERSION}/configure --enable-gpl --enable-nonfree --enable-version3 \
		 --enable-frei0r \
		 --enable-gnutls \
		 --enable-libbs2b \
		 --enable-libcdio \
		 --enable-libdc1394 \
		 --enable-libfreetype \
		 --enable-libgsm \
		 --enable-libmp3lame \
		 --enable-libopencore-amrnb \
		 --enable-libopencore-amrwb \
		 --enable-libopencv \
		 --enable-libopenjpeg \
		 --enable-libopus \
		 --enable-libpulse \
		 --enable-librtmp \
		 --enable-libschroedinger \
		 --enable-libspeex \
		 --enable-libtheora \
		 --enable-libtwolame \
		 --enable-libvo-aacenc \
		 --enable-libvo-amrwbenc \
		 --enable-libvorbis \
		 --enable-libvpx \
		 --enable-libwavpack \
		 --enable-libwebp \
		 --enable-libx264 \
		 --enable-libxvid \
		 --enable-openssl \
		 --enable-x11grab ;
#		 --enable-libx265 \
	then
		echo \`configure\' failed. 1>&2
		exit 1
	fi
	# libfaac -> libfaac is nonfree and --enable-nonfree is not specified.
	#  --enable-libfdk-aac
	# libx265 -> x265 not found
fi

if ! ${SUDO}make -j4 ;
then
	echo \`make\' failed. 1>&2
	exit 1
fi

if ! ${SUDO}make install ;
then
	echo \`make install\' failed. 1>&2
	exit 1
fi

echo Installation done.

exit 0
