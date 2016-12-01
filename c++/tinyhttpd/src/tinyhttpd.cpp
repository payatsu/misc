#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

typedef std::map<std::string, std::string> Field;
typedef std::map<int, std::string> StatusCode;
const char crlf[] = {0x0d, 0x0a, 0x00};
const char emptyline[] = {0x0d, 0x0a, 0x0d, 0x0a, 0x00};

class Socket{
public:
	Socket(int sock): sock_(sock){if(sock_ == -1){throw std::runtime_error(std::strerror(errno));}}
	~Socket(){close(sock_);}
	operator int()const{return sock_;}
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);
	int sock_;
};

Field parse(const std::string& request)
{
	std::istringstream iss(request);
	std::string line;
	int total_lines = 0;
	bool is_body = false;
	Field field;
	while(std::getline(iss, line)){
		if(is_body){
		}else if(total_lines){
			if(line == "\r"){
				is_body = true;
				continue;
			}
			std::string::size_type column_pos = line.find_first_of(": ");
			if(column_pos == std::string::npos){
				std::cerr << "no colon found. invalid request format.: '" << line << '\'' << std::endl;
				continue;
			}
			field[line.substr(0, column_pos)] = line.substr(column_pos + 2);
		}else{
			std::istringstream request_line(line);
			request_line >> field["method"] >> field["uri"] >> field["http_version"];
		}
		total_lines++;
	}
	return field;
}

StatusCode initialize_status_code()
{
	StatusCode status_code;
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
	return status_code;
}

StatusCode status_code = initialize_status_code();

void show(const Field& field)
{
	for(Field::const_iterator it = field.begin(); it != field.end(); ++it){
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

std::string reply(const std::string& request)
{
	Field field = parse(request);
	std::ostringstream response;
	response << field["http_version"] << ' ' << 200 << ' ' << status_code[200] << crlf;
	response << "Content-Type: image/png" << crlf;
	std::ifstream img("./img/HSV1.png", std::ios::binary | std::ios::ate);
	response << "Content-Length: " << img.tellg() << crlf << crlf;
	img.seekg(0, std::ios::beg);
	response << std::string(std::istreambuf_iterator<char>(img), std::istreambuf_iterator<char>());
	return response.str();
}

std::string receive(int conn)
{
	const int request_limit = 512;
	char message[1024] = {};
	int len = 0;
	std::string request;
	while(true){
		if((len = recv(conn, message, sizeof(message), 0)) == -1){
			throw std::domain_error(std::string("recv() failed.: ") + std::strerror(errno));
		}
		request += std::string(message, len);
		if(request_limit < request.size()){
			throw std::domain_error("too long http request.");
		}
		if(std::string::npos != request.find(emptyline)){
			break;
		}
	}
	return request;
}

int main(int argc, char* argv[])
{
	const int port = 1 < argc ? std::atoi(argv[1]) : 8080;
	Socket sock(socket(AF_INET, SOCK_STREAM, 0));
	sockaddr_in addr = {AF_INET, htons(port), INADDR_ANY, {}};
	if(bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) == -1){
		throw std::runtime_error(std::string("bind() failed.: ") + std::strerror(errno));
	}
	if(listen(sock, 1) == -1){
		throw std::runtime_error(std::string("listen() failed.: ") + std::strerror(errno));
	}
	while(true){
		Socket conn(accept(sock, NULL, NULL));
		std::string request = receive(conn);
		std::string response = reply(request);
		send(conn, response.c_str(), response.size(), 0);
	}
	return 0;
}
