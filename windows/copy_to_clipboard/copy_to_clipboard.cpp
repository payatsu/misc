/**
 *
 * @code
 $ i686-pc-cygwin-g++ -mwindows -o set_clipboard.exe set_clipboard.cpp
 * @endcode
 */

#include <string>
#include <windows.h>

int main(int argc, char* argv[])
{
	std::string str;
	for(int i = 1; i < argc; ++i){
		str += "<file:";
		str += argv[i];
		str += ">\r\n";
	}

	HGLOBAL hMem;
	LPTSTR lpBuff;

	if((hMem = GlobalAlloc((GHND | GMEM_SHARE), str.size() + 1)) != NULL){
		if((lpBuff = (LPTSTR)GlobalLock(hMem)) != NULL){
			lstrcpy(lpBuff, str.data());
			GlobalUnlock(hMem);
			if(OpenClipboard(NULL)){
				EmptyClipboard();
				SetClipboardData(CF_TEXT, hMem);
				CloseClipboard();
			}
		}else{
			GlobalFree(hMem);
		}
	}
	return 0;
}
