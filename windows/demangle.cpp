#include <iostream>
#include <libmangle.h>
#include <memory>

int main()
{
	std::shared_ptr<libmangle_gc_context_t> ctx(libmangle_generate_gc(), [](libmangle_gc_context_t* c){libmangle_release_gc(c);});
	std::string line;
	while(std::getline(std::cin, line)){
		std::shared_ptr<char> demangled(libmangle_sprint_decl(libmangle_decode_ms_name(ctx.get(), line.c_str())));
		if(demangled){
			std::cout << demangled << std::endl;
		}
	}
	return 0;
}
