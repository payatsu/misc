#!/usr/bin/python2.7

import cv2, numpy, os

height = 1080
width = 1920
img = numpy.zeros((height, width, 3))

for i in range(height):
	for j in range(width):
		img[i, j, 0] = img[i, j, 1] = img[i, j, 2] = j * 255 / width

# cv2.imshow('img', img)
cv2.imwrite(os.getcwd() + os.sep + 'gradation.bmp', img)
