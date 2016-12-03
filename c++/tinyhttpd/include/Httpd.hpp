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
	static Field parse(const std::string& request);
	static std::string process(const std::string& request);
	std::string receive()const;
	void send(const std::string& reply)const;
	static const char* get_status_code_string(unsigned int code);
	static const char* get_mime_type(const std::string& uri);
	static void get_content(const std::string& uri, std::ostringstream& reply);
	static void dump_request(const Field& field);
	static void dump_reply(const std::string& reply);
	Socket sock_;
};

#endif
