#!/bin/sh

width=1920
height=1080
n=4

tmpdir=`mktemp --tmpdir -d`
trap 'rm -rf ${tmpdir}' EXIT HUP INT QUIT TERM

rectangle_width=`expr ${width} / ${n}`
rectangle_height=`expr ${height} / ${n}`
convert -verbose -size ${width}x${height} xc:white -fill black \
	-draw "rectangle `expr ${rectangle_width} \* 0`,`expr ${rectangle_height} \* 0` `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 1`" \
	-draw "rectangle `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 0` `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 1`" \
	-draw "rectangle `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 1` `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 2`" \
	-draw "rectangle `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 1` `expr ${rectangle_width} \* 4`,`expr ${rectangle_height} \* 2`" \
	-draw "rectangle `expr ${rectangle_width} \* 0`,`expr ${rectangle_height} \* 2` `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 3`" \
	-draw "rectangle `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 2` `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 3`" \
	-draw "rectangle `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 3` `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 4`" \
	-draw "rectangle `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 3` `expr ${rectangle_width} \* 4`,`expr ${rectangle_height} \* 4`" \
	${tmpdir}/base.png || return

fmt=%05d
for i in `seq \`expr ${rectangle_height} \* 1\``; do
	num=`printf ${fmt} ${i}`
	convert -verbose ${tmpdir}/base.png -roll +${i}+${i} -fill red -font Ricty-Regular -pointsize 400 -gravity center -annotate 0 ${num} ${tmpdir}/img_${num}.png || return
done
ffmpeg -framerate 120 -i ${tmpdir}/img_${fmt}.png -vcodec h264 -pix_fmt yuv420p -an -q:v 0 -r 120 movie.mp4 || return
