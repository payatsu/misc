#!/bin/sh

start=0
duration=10
frame_rate=60

while getopts s:t:r: option
do
	case ${option} in
		s)
			start=${OPTARG}
			;;
		t)
			duration=${OPTARG}
			;;
		r)
			frame_rate=${OPTARG}
			;;
		\?)
			exit 1
			;;
	esac
done
shift `expr ${OPTIND} - 1`

for in_file in "$@"
do
	out_file=`echo ${in_file} | sed -e 's/\(.*\)\.[^.]\+$/\1_%05d.jpg/'`
	avconv -i ${in_file} -ss ${start} -t ${duration} -r ${frame_rate} -f image2 ${out_file}
done
