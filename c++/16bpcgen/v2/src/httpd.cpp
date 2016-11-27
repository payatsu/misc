#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

typedef std::map<std::string, std::string> Fields;
typedef std::map<int, std::string> StatusCodes;
const char newline[] = {0x0d, 0x0a, 0x00};
const char emptyline[] = {0x0d, 0x0a, 0x0d, 0x0a, 0x00};

class Socket{
public:
	Socket(int sock): sock_(sock)
	{
		if(sock_ == -1){
			throw std::runtime_error(std::strerror(errno));
		}
	}
	~Socket(){close(sock_);}
	operator int()const{return sock_;}
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);
	int sock_;
};

Fields parse(const std::string& request)
{
	std::istringstream iss(request);
	std::string line;
	int total_lines = 0;
	bool is_body = false;
	Fields fields;
	while(std::getline(iss, line)){
		if(is_body){
		}else if(total_lines){
			if(line == "\r"){
				is_body = true;
				continue;
			}
			std::string::size_type column_pos = line.find_first_of(':');
			if(column_pos == std::string::npos){
				std::cerr << "invalid request format: '" << line << '\'' << std::endl;
				continue;
			}
			std::string name  = line.substr(0, column_pos);
			std::string param = line.substr(column_pos + 2);
			fields[name] = param;
		}else{
			std::istringstream request_line(line);
			std::string method;
			std::string uri;
			std::string http_version;
			request_line >> method >> uri >> http_version;
			fields["method"]       = method;
			fields["uri"]          = uri;
			fields["http_version"] = http_version;
		}
		total_lines++;
	}
	return fields;
}

std::string reply(const std::string& request)
{
	Fields fields = parse(request);
	for(Fields::iterator it = fields.begin(); it != fields.end(); ++it){
		std::cout << it->first << ": " << it->second << std::endl;
	}
	StatusCodes status_code;
	status_code[100] = "Continue";
	status_code[101] = "Switching Protocols";
	status_code[200] = "OK";
	status_code[201] = "Created";
	status_code[202] = "Accepted";
	status_code[203] = "Non-Authoritative Information";
	status_code[204] = "No Content";
	status_code[205] = "Reset Content";
	status_code[206] = "Partial Content";
	status_code[300] = "Multiple Choices";
	status_code[301] = "Moved Permanently";
	status_code[302] = "Moved Temporarily";
	status_code[303] = "See Other";
	status_code[304] = "Not Modified";
	status_code[307] = "Temporary Redirect";
	status_code[400] = "Bad Request";
	status_code[401] = "Unauthorized";
	status_code[402] = "Payment Required";
	status_code[403] = "Forbidden";
	status_code[404] = "Not Found";
	status_code[405] = "Method Not Allowed";
	status_code[406] = "Not Acceptable";
	status_code[407] = "Proxy Authentication Required";
	status_code[408] = "Request Time-out";
	status_code[409] = "Conflict";
	status_code[500] = "Internal Server Error";
	status_code[501] = "Not Implemented";
	status_code[502] = "Bad Gateway";
	status_code[503] = "Service Unavailable";
	status_code[504] = "Gateway Time-out";
	status_code[505] = "HTTP Version not supported";
	std::ostringstream response;
	response << fields["http_version"] << ' ' << 200 << ' ' << status_code[200] << newline;
	response << "Content-Type: image/png" << newline;
	std::ifstream img("./img/HSV1.png", std::ios::binary);
	img.seekg(0, img.end);
	int length = img.tellg();
	img.seekg(0, img.beg);
	response << "Content-Length: " << length << newline;
	response << newline;

	char buffer[1024] = {};
	while(!img.eof()){
		img.read(buffer, sizeof(buffer));
		response.write(buffer, img.gcount());
	}
	return response.str();
}

int main(int argc, char* argv[])
{
	const int port = 1 < argc ? std::atoi(argv[1]) : 8080;
	Socket sock(socket(AF_INET, SOCK_STREAM, 0));
	sockaddr_in addr = {
		AF_INET,
		htons(port),
		INADDR_ANY,
		{}
	};
	if(bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) == -1){
		throw std::runtime_error(std::string("bind() failed.: ") + std::strerror(errno));
	}
	if(listen(sock, 1) == -1){
		throw std::runtime_error(std::string("listen() failed.: ") + std::strerror(errno));
	}
	while(true){
		Socket conn(accept(sock, NULL, NULL));
		const int request_limit = 512;
		char msg[1024] = {};
		int len = 0;
		std::string request;
		while(true){
			if((len = recv(conn, msg, sizeof(msg), 0)) == -1){
				throw std::domain_error(std::string("recv() failed.: ") + std::strerror(errno));
			}
			request += std::string(msg, len);
			if(request_limit < request.size()){
				throw std::domain_error("too long http request.");
			}
			std::string::size_type pos = request.find(emptyline);
			if(pos != std::string::npos){
				break;
			}
		}
		std::string response = reply(request);
		send(conn, response.c_str(), response.size(), 0);
	}
	return 0;
}
