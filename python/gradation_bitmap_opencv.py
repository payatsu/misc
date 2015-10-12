#!/usr/bin/python2.7

import cv2, numpy

height = 1080
width = 1920

cv2.imwrite('gradation.bmp',
			numpy.tile(numpy.arange(width)*255/width, height).reshape(height, width))

# img = numpy.zeros((height, width, 3))
# for j in range(width):
# 	img[:, j, :] = j * 255 / width
