#ifndef _16BPCGEN_GETOPT_H_
#define _16BPCGEN_GETOPT_H_

#include <algorithm>
#include <cstring>
#include <map>
#include <string>

typedef std::map<std::string, std::string> Store;
bool is_equal(char c){return c == '=';}

inline Store getopt(int argc, char* argv[])
{
	Store store;
	for(int i = 1; i < argc; ++i){
		if(std::count_if(argv[i], argv[i] + std::strlen(argv[i]), is_equal) == 1){
			const std::string::size_type idx = std::string(argv[i]).find('=');
			const std::string key(argv[i], idx);
			const std::string value(argv[i] + idx + 1);
			store[key] = value;
		}
	}
	return store;
}

#endif
