/**
 * @file
 * @brief Network frontend of 16bpcgen.
 */

#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "getopt.hpp"

int main(int argc, char* argv[])
{
	Store store = getopt(argc, argv);
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		perror(argv[0]);
		return -1;
	}
	sockaddr_in addr = {
		AF_INET,
		htons(5000),
		INADDR_ANY,
		{}
	};
	if(bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) == -1){
		perror(argv[0]);
		return -1;
	}
	if(listen(sock, 1) == -1){
		perror(argv[0]);
		return -1;
	}

	while(true){
		int conn = accept(sock, nullptr, nullptr);
		if(conn == -1){
			perror(argv[0]);
			return -1;
		}

		char msg[1024] = {};
		int len = 0;
		if((len = recv(conn, msg, 1024, 0)) == -1){
			perror(argv[0]);
			return -1;
		}
		std::cout.write(msg, len);
		close(conn);
	}

	close(sock);
	return 0;
}
