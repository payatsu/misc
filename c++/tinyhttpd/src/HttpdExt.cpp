#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include "HttpdExt.hpp"
#include "Image.hpp"

struct NonDigitEliminator{
	char operator()(const char c)const
	{
		if(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') || c == '\n'){
			return c;
		}else{
			return ' ';
		}
	}
};

struct LineFeedDetector{
	bool operator()(const char c)const{return c == '\n';}
};

void HttpdExt::process_post_data(HttpdExt::Field& field)const
{
	std::string buffer;
	std::transform(field["file"].begin(), field["file"].end(), std::back_inserter(buffer), NonDigitEliminator());
	if(!has_digit_consistency(buffer) || !has_component_consistency(buffer)){
		std::cerr << "invalid data." << std::endl;
		return;
	}

	std::istringstream data(buffer);
	std::string line;
	int i = 0;
	Image result(0, 0);
	while(std::getline(data, line)){
		std::istringstream iss(line);
		iss >> std::hex;
		uint16_t value;
		Image line_buffer(0, 0);
		switch(i % 3){
		case 0:{
			while(iss >> value){
				Image tmp(1, 1);
				tmp[0][0] = Image::pixel_type(value, 0, 0) << 4;
				line_buffer = line_buffer(tmp, Image::ORI_HORI);
			}
			break;
		}
		case 1:{
			while(iss >> value){
				Image tmp(1, 1);
				tmp[0][0] = Image::pixel_type(0, value, 0) << 4;
				line_buffer = line_buffer(tmp, Image::ORI_HORI);
			}
			break;
		}
		case 2:{
			while(iss >> value){
				Image tmp(1, 1);
				tmp[0][0] = Image::pixel_type(0, 0, value) << 4;
				line_buffer = line_buffer(tmp, Image::ORI_HORI);
			}
			break;
		}
		default:
			break;
		}
		result = result(line_buffer, Image::ORI_VERT);
		++i;
	}
	result >> "./res/result.png";
}

bool HttpdExt::has_digit_consistency(const std::string& data)const
{
	std::istringstream iss(data);
	std::string number;
	iss >> number;
	const std::string::size_type digit = number.size();
	while(iss >> number){
		if(digit != number.size()){
			return false;
		}
	}
	return true;
}

bool HttpdExt::has_component_consistency(const std::string& data)const
{
	return !(std::count_if(data.begin(), data.end(), LineFeedDetector()) % 3);
}
