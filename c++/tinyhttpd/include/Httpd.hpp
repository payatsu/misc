#ifndef TINYHTTPD_HTTPD_HPP_
#define TINYHTTPD_HTTPD_HPP_

#include <map>
#include <string>
#include "Socket.hpp"

class Httpd{
public:
	typedef std::map<std::string, std::string> Field;
	explicit Httpd(int sock): sock_(sock){}
	void run()const;
	virtual ~Httpd(){}
private:
	static const char crlf[];
	static const char emptyline[];
	Field parse(const std::string& request)const;
	std::string process(const std::string& request)const;
	std::string receive()const;
	void send(const std::string& reply)const;
	const char* get_status_code_string(unsigned int code)const;
	const char* get_mime_type(const std::string& uri)const;
	void prepare_content(Field& field, std::ostringstream& reply)const;
	virtual void process_post_data(Field& field)const{static_cast<void>(field);} // do nothing.
	void dump_request(const Field& field)const;
	void dump_reply(const std::string& reply)const;
	Socket sock_;
};

#endif
