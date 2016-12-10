#ifndef TINYHTTPD_HTTPDEXT_HPP_
#define TINYHTTPD_HTTPDEXT_HPP_

#include "Httpd.hpp"

class HttpdExt: public Httpd{
public:
	explicit HttpdExt(int sock): Httpd(sock){}
	virtual ~HttpdExt(){}
private:
	virtual void process_post_data(Field& field)const;
	bool has_digit_consistency(const std::string& data)const;
	bool has_component_consistency(const std::string& data)const;
};

#endif
