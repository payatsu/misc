#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	const row_t    height = store["height"] == "" ? 1080 : atoi(store["height"].c_str());
	const column_t width  = store["width"]  == "" ? 1920 : atoi(store["width"].c_str());

	if(store["builtins"] != ""){
		generate_builtin_patterns(width, height);
		return 0;
	}

	Image image(width, height);
	if(store["input"] == "-"){
		image <<= std::cin;
	}else if(store["input"] != ""){
		std::ifstream ifs(store["input"].c_str());
		image <<= ifs;
	}

	if(store["output"] == ""){
		store["output"] = "out";
	}
	image >> store["output"];
	return 0;
}
