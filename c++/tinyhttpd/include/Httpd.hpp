#ifndef TINYHTTPD_HTTPD_HPP_
#define TINYHTTPD_HTTPD_HPP_

#include <map>
#include <string>
#include "Socket.hpp"

class Httpd{
public:
	explicit Httpd(int sock): sock_(sock){}
	void run()const;
private:
	typedef std::map<std::string, std::string> Field;
	static const char crlf[];
	static const char emptyline[];
	Field parse(const std::string& request)const;
	std::string process(const std::string& request)const;
	std::string receive()const;
	void send(const std::string& reply)const;
	const char* get_status_code_string(unsigned int code)const;
	void dump_request(const Field& field)const;
	Socket sock_;
};

#endif
