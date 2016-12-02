#include <cstdlib>
#include <netinet/in.h>
#include "Httpd.hpp"

int main(int argc, char* argv[])
{
	const int port = 1 < argc ? std::atoi(argv[1]) : 8080;
	Socket sock(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr = {AF_INET, htons(port), INADDR_ANY, {}};
	sock.bind(reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in));
	sock.listen(1);
	while(true){
		Httpd(sock.accept(NULL, NULL)).run();
	}
	return 0;
}
