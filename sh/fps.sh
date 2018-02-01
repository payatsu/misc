#!/bin/sh

width=1920
height=1080

n=4

tmpdir=.
# tmpdir=`mktemp --tmpdir -d`
# trap 'rm -rf ${tmpdir}' EXIT HUP INT QUIT TERM

rectangle_width=`expr ${width} / ${n}`
rectangle_height=`expr ${height} / ${n}`

convert -size ${width}x${height} xc:white -fill black \
	-draw "rectangle `expr ${rectangle_width} \* 0`,`expr ${rectangle_height} \* 0` `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 1`" \
	-draw "rectangle `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 0` `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 1`" \
	-draw "rectangle `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 1` `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 2`" \
	-draw "rectangle `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 1` `expr ${rectangle_width} \* 4`,`expr ${rectangle_height} \* 2`" \
	-draw "rectangle `expr ${rectangle_width} \* 0`,`expr ${rectangle_height} \* 2` `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 3`" \
	-draw "rectangle `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 2` `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 3`" \
	-draw "rectangle `expr ${rectangle_width} \* 1`,`expr ${rectangle_height} \* 3` `expr ${rectangle_width} \* 2`,`expr ${rectangle_height} \* 4`" \
	-draw "rectangle `expr ${rectangle_width} \* 3`,`expr ${rectangle_height} \* 3` `expr ${rectangle_width} \* 4`,`expr ${rectangle_height} \* 4`" \
	${tmpdir}/base.png

image_name_format=img_%06d

for i in `seq ${rectangle_height}`; do
	name=`printf ${image_name_format} ${i}`
	convert ${tmpdir}/base.png -roll +${i}+${i} ${name}.png
done

ffmpeg -framerate 120 -i ${image_name_format}.png -vcodec h264 -pix_fmt yuv444p -an -qscale 0 -r 120 movie.mp4
