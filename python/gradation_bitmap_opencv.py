#!/usr/bin/python2.7

import cv2, numpy

height = 1080
width = 1920

cv2.imwrite('gradation.bmp',
			numpy.tile(numpy.arange(width)*255/width, height).reshape(height, width))
