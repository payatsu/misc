#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Httpd.hpp"
#include "misc.hpp"

const char Httpd::crlf[] = {0x0d, 0x0a, 0x00};
const char Httpd::emptyline[] = {0x0d, 0x0a, 0x0d, 0x0a, 0x00};

void Httpd::run()const
{
	const std::string request = receive();
	const std::string reply = process(request);
	send(reply);
}

Httpd::Field Httpd::parse(const std::string& request)
{
	std::istringstream iss(request);
	std::string line;
	int total_lines = 0;
	Field field;
	while(std::getline(iss, line)){
		if(total_lines || line.find(":") != std::string::npos){
			if(line == "\x0d"){
				field["body"] = std::string(std::istreambuf_iterator<char>(iss), std::istreambuf_iterator<char>());
				break;
			}
			const std::string::size_type column_pos = line.find(": ");
			if(column_pos == std::string::npos){
				std::cerr << "no colon found. invalid request format.: '" << line << '\'' << std::endl;
				continue;
			}
			field[line.substr(0, column_pos)] = line.substr(column_pos + std::strlen(": "));
		}else{
			std::istringstream request_line(line);
			request_line >> field["method"] >> field["uri"] >> field["http_version"];
		}
		total_lines++;
	}
	return field;
}

std::string Httpd::process(const std::string& request)
{
	Field field = parse(request);
	dump_request(field);
	std::ostringstream reply;
	reply << field["http_version"] << ' ' << 200 << ' ' << get_status_code_string(200) << crlf;
	reply << "Content-Type: " << get_mime_type(field["uri"]) << crlf;
	get_content(field, reply);
	return reply.str();
}

std::string Httpd::receive()const
{
	std::string request;
	while(true){
		char message[64*1024];
		request += std::string(message, sock_.recv(message, sizeof(message), 0));
		const std::string::size_type pos = request.find(emptyline);
		if(pos != std::string::npos){
			Field field = parse(request);
			if(field.find("Content-Length") != field.end()){
				const std::size_t remainder = std::atoi(field["Content-Length"].c_str()) - (request.size() - pos - std::strlen(emptyline));
				std::size_t received = 0;
				while(received < remainder){
					const int len = sock_.recv(message, sizeof(message), 0);
					request += std::string(message, len);
					received += len;
				}
			}
			break;
		}
	}
	return request;
}

void Httpd::send(const std::string& reply)const
{
	sock_.send(reply.c_str(), reply.size(), 0);
}

const char* Httpd::get_status_code_string(unsigned int code)
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

const char* Httpd::get_mime_type(const std::string& uri)
{
	const std::string::size_type pos = uri.find_last_of(".");
	if(pos == std::string::npos || pos == uri.size() - 1){
		return "";
	}else{
		const std::string suffix = uri.substr(pos + 1);
		if(suffix == "html" || suffix == "htm"){
			return "text/html";
		}else if(suffix == "tif" || suffix == "tiff"){
			return "image/tiff";
		}else if(suffix == "png"){
			return "image/png";
		}else{
			return "";
		}
	}
}

void Httpd::get_content(Httpd::Field& field, std::ostringstream& reply)
{
	const std::string filename = field["uri"] == "/" ? "/index.html" : field["uri"];
	std::ifstream content(("./res" + filename).c_str(), std::ios::binary | std::ios::ate);
	if(!content.good()){
		return;
	}
	reply << "Content-Length: " << content.tellg() << crlf << crlf;
	content.seekg(0, std::ios::beg);
	reply << std::string(std::istreambuf_iterator<char>(content), std::istreambuf_iterator<char>());
	if(field["method"] == "GET"){
	}else if(field["method"] == "POST"){
		const char* param_name = "multipart/form-data; boundary=";
		std::string boundary = field["Content-Type"].substr(field["Content-Type"].find(param_name) + std::strlen(param_name));
		boundary.erase(boundary.find_last_of('\x0d'));
		std::string body = field["body"].substr(field["body"].find(crlf) + std::strlen(crlf));
		Field post_field;
		while(true){
			const std::string::size_type next_pos = body.find("--" + boundary);
			if(next_pos == std::string::npos){
				break;
			}
			Field post = parse(body.substr(0, next_pos));
			const char* param_name2 = "form-data; name=\"";
			post["body"].erase(post["body"].rfind(crlf), std::strlen(crlf));
			post_field[post["Content-Disposition"].substr(std::strlen(param_name2), post["Content-Disposition"].find("\"", std::strlen(param_name2)) - std::strlen(param_name2))] = post["body"];
			body.erase(0, body.find(crlf, next_pos) + std::strlen(crlf));
		}
		generate_image(post_field["file"]);
	}
}

void Httpd::dump_request(const Field& field)
{
	for(Httpd::Field::const_iterator it = field.begin(); it != field.end(); ++it){
		std::cout << it->first << ": " << it->second << std::endl;
	}
}

void Httpd::dump_reply(const std::string& reply)
{
	std::cout << reply << std::endl;
}
