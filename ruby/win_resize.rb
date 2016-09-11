#!/usr/bin/ruby

# -*- coding: utf-8 -*-

# [Prerequisites]
# #gem install ruby-opencv

require 'rubygems'
require 'opencv'

window = OpenCV::GUI::Window.new 'Main window'
window.show OpenCV::IplImage.new 100, 100
# capture = OpenCV::CvCapture.open
detector = OpenCV::CvHaarClassifierCascade::load '/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml'

loop do
#	image = capture.query
	image = OpenCV::IplImage::load 'sample.png', 1
	image = image.resize OpenCV::CvSize.new 640, 360
	detector.detect_objects(image).each do |region|
		image.rectangle! region.top_left, region.bottom_right, :color => OpenCV::CvColor::Red
	end
	window.show image
	break if OpenCV::GUI::wait_key 100
end
