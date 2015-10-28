#!/bin/sh

SUDO='sudo '

# ${SUDO}apt-get update
${SUDO}apt-get install --yes yasm
${SUDO}apt-get install --yes \
	   libladspa-ocaml-dev \
	   libiec61883-dev \
	   libass-dev \
	   libbluray-dev \
	   libbs2b-dev \
	   libcaca-dev \


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

SRC_DIR=/usr/local/src/ffmpeg/
${SUDO}mkdir -p `dirname ${SRC_DIR}`
cd `dirname ${SRC_DIR}`
if [ -d ffmpeg ] ;
then
	cd ./ffmpeg
	${SUDO}git pull
	cd ../
else
	${SUDO}git clone git://source.ffmpeg.org/ffmpeg.git
fi

${SUDO}mkdir -p build
cd build
${SUDO}../ffmpeg/configure --enable-gpl --enable-version3 --enable-nonfree \
	   --enable-avresample \
	   --enable-fontconfig \
	   --enable-frei0r \
	   --enable-gnutls \
	   --enable-ladspa \
	   --enable-libaacplus \
	   --enable-libass \
	   --enable-libbluray \
	   --enable-libbs2b \
	   --enable-libcaca \
	   --enable-libcelt \
	   --enable-libcdio \
	   --enable-libdc1394 \
	   --enable-libdcadec \
	   --enable-libfaac \
	   --enable-libfdk-aac \
	   --enable-libflite \
	   --enable-libfreetype \
	   --enable-libfribidi \
	   --enable-libgme \
	   --enable-libgsm \
	   --enable-libiec61883 \
	   --enable-libkvazaar \
	   --enable-libmfx \
	   --enable-libmodplug \
	   --enable-libmp3lame \
	   --enable-libnut \
	   --enable-libopencore-amrnb \
	   --enable-libopencore-amrwb \
	   --enable-libopencv \
	   --enable-libopenh264 \
	   --enable-libopenjpeg \
	   --enable-libopus \
	   --enable-libpulse \
	   --enable-libquvi \
	   --enable-librtmp \
	   --enable-libschroedinger \
	   --enable-libshine \
	   --enable-libsmbclient \
	   --enable-libsnappy \
	   --enable-libsoxr \
	   --enable-libspeex \
	   --enable-libssh \
	   --enable-libstagefright-h264 \
	   --enable-libtheora \
	   --enable-libtwolame \
	   --enable-libutvideo \
	   --enable-libv4l2 \
	   --enable-libvidstab \
	   --enable-libvo-aacenc \
	   --enable-libvo-amrwbenc \
	   --enable-libvorbis \
	   --enable-libvpx \
	   --enable-libwavpack \
	   --enable-libwebp \
	   --enable-libx264 \
	   --enable-libx265 \
	   --enable-libxavs \
	   --enable-libxcb \
	   --enable-libxcb-shm \
	   --enable-libxcb-xfixes \
	   --enable-libxcb-shape \
	   --enable-libxvid \
	   --enable-libzmq \
	   --enable-libzvbi \
	   --disable-lzma \
	   --enable-mmal \
	   --enable-nvenc \
	   --enable-openal \
	   --enable-opencl \
	   --enable-opengl \
	   --enable-openssl

#   --enable-encoder=NAME    enable encoder NAME
#   --enable-decoder=NAME    enable decoder NAME
#   --enable-hwaccel=NAME    enable hwaccel NAME
#   --enable-muxer=NAME      enable muxer NAME
#   --enable-demuxer=NAME    enable demuxer NAME
#   --enable-parser=NAME     enable parser NAME
#   --enable-bsf=NAME        enable bitstream filter NAME
#   --enable-protocol=NAME   enable protocol NAME
#   --enable-indev=NAME      enable input device NAME
#   --enable-outdev=NAME     enable output device NAME
#   --enable-filter=NAME     enable filter NAME
