#!/usr/bin/python
# -*-coding: utf-8 -*-

import cv2

def resize(in_file, out_file):
	# アスペクト比を保ったままリサイズする。

	img = cv2.imread(in_file)
	cv2.imwrite(out_file, cv2.resize(img, (1000, int(1000.0*img.shape[0]/img.shape[1]))))

if __name__ == "__main__":
	from sys import argv
	from os.path import splitext

	for target in argv[1:]:
		path, ext = splitext(target)
		resize(target, path + '_resized' + ext)
