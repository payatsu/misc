#ifndef BPCGEN_LOG_HPP_
#define BPCGEN_LOG_HPP_

#include <ostream>

class Log{
public:
	enum Level{
		LOG_CRIT,
		LOG_ERR,
		LOG_WARN,
		LOG_NOTICE,
		LOG_INFO,
		LOG_DEBUG
	};
	Log(std::ostream& os, Level level): os_(os), level_(level){}
	virtual ~Log() = 0;
	template <typename T>
	Log& operator<<(const T& rhs){os_ << rhs; return *this;}
	Log& operator<<(std::ostream& (*rhs)(std::ostream&)){os_ << rhs; return *this;}
private:
	std::ostream& os_;
	Level level_;
};

class Crit: public Log{
public:
	Crit(std::ostream& os): Log(os, LOG_CRIT){}
	virtual ~Crit(){}
};

class Err{};
class Warn{};
class Notice{};
class Info{};
class Debug{};

#endif
