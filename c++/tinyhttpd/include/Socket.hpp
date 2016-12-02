#ifndef TINYHTTPD_SOCKET_HPP_
#define TINYHTTPD_SOCKET_HPP_

#include <sys/socket.h>

class Socket{
public:
	explicit Socket(int sock);
	Socket(int domain, int type, int protocol);
	~Socket();
	int accept(sockaddr* addr, unsigned int* addrlen)const;
	void bind(const sockaddr* addr, unsigned int addrlen)const;
	void listen(int backlog)const;
	ssize_t recv(void* buf, std::size_t len, int flags)const;
	ssize_t send(const void* buf, std::size_t len, int flags)const;
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);
	int sock_;
};

#endif
