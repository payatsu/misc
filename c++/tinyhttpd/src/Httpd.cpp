#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Httpd.hpp"

const char Httpd::crlf[] = {0x0d, 0x0a, 0x00};
const char Httpd::emptyline[] = {0x0d, 0x0a, 0x0d, 0x0a, 0x00};

void Httpd::run()const
{
	std::string request = receive();
	std::string reply = process(request);
	send(reply);
}

Httpd::Field Httpd::parse(const std::string& request)const
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

std::string Httpd::process(const std::string& request)const
{
	Field field = parse(request);
	std::ostringstream reply;
	reply << field["http_version"] << ' ' << 200 << ' ' << get_status_code_string(200) << crlf;
	reply << "Content-Type: image/png" << crlf;
	std::ifstream img("./img/HSV1.png", std::ios::binary | std::ios::ate);
	reply << "Content-Length: " << img.tellg() << crlf << crlf;
	img.seekg(0, std::ios::beg);
	reply << std::string(std::istreambuf_iterator<char>(img), std::istreambuf_iterator<char>());
	return reply.str();
}

std::string Httpd::receive()const
{
	const int request_limit = 512;
	char message[1024] = {};
	int len = 0;
	std::string request;
	while(true){
		if((len = sock_.recv(message, sizeof(message), 0)) == -1){
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

void Httpd::send(const std::string& reply)const
{
	sock_.send(reply.c_str(), reply.size(), 0);
}

const char* Httpd::get_status_code_string(unsigned int code)const
{
	switch(code){
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Moved Temporarily";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 307: return "Temporary Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Time-out";
	case 409: return "Conflict";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Time-out";
	case 505: return "HTTP Version not supported";
	default:  return "Internal Server Error";
	}
}

void Httpd::dump_request(const Field& field)const
{
	for(Httpd::Field::const_iterator it = field.begin(); it != field.end(); ++it){
		std::cout << it->first << ": " << it->second << std::endl;
	}
}
