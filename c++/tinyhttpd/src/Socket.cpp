#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include "Socket.hpp"

Socket::Socket(int sock): sock_(sock)
{
	if(sock_ == -1){
		throw std::runtime_error(std::strerror(errno));
	}
}

Socket::Socket(int domain, int type, int protocol): sock_(-1)
{
	sock_ = ::socket(domain, type, protocol);
	if(sock_ == -1){
		throw std::runtime_error(std::strerror(errno));
	}
}

Socket::~Socket(){close(sock_);}

int Socket::accept(sockaddr* addr, unsigned int* addrlen)const
{
	return ::accept(sock_, addr, addrlen);
}

void Socket::bind(const sockaddr* addr, unsigned int addrlen)const
{
	if(::bind(sock_, addr, addrlen) == -1){
		throw std::runtime_error(std::string("bind() failed.: ") + std::strerror(errno));
	}
}

void Socket::listen(int backlog)const
{
	if(::listen(sock_, backlog) == -1){
		throw std::runtime_error(std::string("listen() failed.: ") + std::strerror(errno));
	}
}

ssize_t Socket::recv(void* buf, std::size_t len, int flags)const
{
	ssize_t ret = ::recv(sock_, buf, len, flags);
	if(ret == -1){
		throw std::domain_error(std::string("recv() failed.: ") + std::strerror(errno));
	}
	return ret;
}

ssize_t Socket::send(const void* buf, std::size_t len, int flags)const
{
	return ::send(sock_, buf, len, flags);
}
