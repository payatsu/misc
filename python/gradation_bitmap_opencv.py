#!/usr/bin/python3

import cv2, numpy, os

height = 1080
max_grad = 255
img = numpy.zeros((height, max_grad+1, 3))

for i in range(height):
	for j in range(max_grad+1):
		img[i, j, 0] = j

cv2.imwrite(os.getcwd() + os.sep + 'gradation.bmp', img)
