/**
 * @brief web browser switcher
 * @details
 * ## How to Compile
 *   $ x86_64-w64-mingw32-g++ -std=c++17 -Wall -Wextra -O3 -static-libgcc -static-libstdc++ -mwindows browser_switcher.cpp -o browser_switcher.exe
 */

#include <cstdlib>
#include <fstream> // XXX: can't use <filesystem> yet on MinGW-w64, so use <fstream> alternatively.
#include <regex>

const char* const chrome   = R"(C:\Program Files (x86)\Google\Chrome\Application\chrome.exe)";
const char* const firefox  = R"(C:\Program Files\Mozilla Firefox\firefox.exe)";
const char* const iexplore = R"(C:\Program Files\internet explorer\iexplore.exe)";

inline std::string select_browser(const std::string& url)
{
	for(const std::regex& regex: std::initializer_list<std::regex>{R"(\bC:\\)"}){
		if(std::regex_search(url, regex)){
			return iexplore;
		}
	}

	const char* favorite_browser = iexplore;
	for(const char* browser: {chrome, firefox}){
		if(std::ifstream(browser)){
			favorite_browser = browser;
			break;
		}
	}
	return favorite_browser;
}

int main(int argc, const char* argv[])
{
	std::string args;
	for(int i = 1; i < argc; ++i){
		args += ' ';
		args += argv[i];
	}
	std::system(('"' + select_browser(args) + '"' + args).c_str());
	return 0;
}
