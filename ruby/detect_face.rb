#!/usr/bin/ruby

# -*- coding: utf-8 -*-

# [Prerequisites]
# #gem install ruby-opencv

require 'rubygems'
require 'opencv'

if ARGV.size != 1
	puts "Usage: ruby ${__FILE__} Image"
	exit
end

image = nil
image_laugh = nil
begin
	image = OpenCV::IplImage.load ARGV[0], 1
	image_laugh = OpenCV::IplImage.load './laugh.png', 1
rescue
	puts 'Could not open or find the image.'
	exit
end

detector = OpenCV::CvHaarClassifierCascade::load '/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml'
detector.detect_objects(image).each do |region|
	resized_image = image_laugh.resize region
	image.set_roi region
	(resized_image.rows * resized_image.cols).times do |i|
		if resized_image[i][0].to_i > 0 or resized_image[i][1].to_i > 0 or resized_image[i][2].to_i > 0
			image[i] = resized_image[i]
		end
	end
	image.reset_roi
end

window = OpenCV::GUI::Window.new 'laugh.rb'
window.show image
OpenCV::GUI::wait_key

