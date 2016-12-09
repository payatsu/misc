#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
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

void generate_image(const std::string& str)
{
	std::string buffer;
	std::transform(str.begin(), str.end(), std::back_inserter(buffer), NonDigitEliminator());

	std::istringstream data(buffer);
	std::string line;
	std::vector<std::vector<unsigned int> > values;
	while(std::getline(data, line)){
		if(line == ""){
			continue;
		}
		std::istringstream iss(line);
		iss >> std::hex;
		values.push_back(std::vector<unsigned int>());
		std::back_insert_iterator<std::vector<unsigned int> > it(values.back());
		std::istream_iterator<unsigned int> isi(iss);
		std::istream_iterator<unsigned int> last;
		for(; isi != last; ++isi){
			*it = *isi;
		}
	}
	try{
		const std::size_t elems = values.at(0).size();
		for(std::vector<std::vector<unsigned int> >::iterator it = values.begin(); it != values.end(); ++it){
			if(it->size() != elems){
				std::cerr << "invalid input." << std::endl;
				return;
			}
		}
		Image img(values.at(0).size(), values.size()/3);
		for(std::size_t i = 0; i < values.size(); i += 3){
			for(std::size_t j = 0; j < values.at(0).size(); ++j){
				img[i][j].R(values.at(i    ).at(j));
				img[i][j].G(values.at(i + 1).at(j));
				img[i][j].B(values.at(i + 2).at(j));
			}
		}
		img >> "./res/result.png";
	}catch(const std::out_of_range& err){
		std::cerr <<err.what() << std::endl;
		return;
	}
}

