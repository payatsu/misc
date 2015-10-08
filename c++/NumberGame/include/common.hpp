#ifndef COMMON_HPP_
#define COMMON_HPP_

#include<ostream>
#include<iomanip>

typedef unsigned int digit_t;
typedef unsigned char base_t;

constexpr digit_t DIGIT = 3u;
constexpr base_t BASE = 10;
constexpr const char* PROMPT = ">";
constexpr const char* INVALID_INPUT = "Invalid input.";
constexpr const char* BYE = "Bye!";
constexpr const char* CONGRATULATIONS = "Congratulations, you are collect!";

struct number{
	base_t number_[DIGIT];
	operator int()const
	{
		int n = 0;
		for(digit_t i = 0 ; i < DIGIT ; i++){
			int base = 1;
			for(digit_t j = 0 ; j < DIGIT - i - 1 ; j++){
				base *= BASE;
			}
			n += base*number_[i];
		}
		return n;
	}
};

std::ostream& operator<<(std::ostream& os, const number& n)
{
	return os << std::setw(DIGIT) << std::setfill('0') << (int)n;
}

class generator{
	bool check_list_[BASE];
	std::mt19937& engine_;
public:
	explicit generator(std::mt19937& e): engine_(e)
	{
		std::fill(check_list_, check_list_+BASE, false);
	}
	base_t operator()()
	{
		base_t tmp;
		do{
			tmp = std::uniform_int_distribution<base_t>{0, BASE-1}(engine_);
		}while(check_list_[tmp] || !(check_list_[tmp]=true));
		return tmp;
	}
};

class checker{
	bool check_list_[BASE];
public:
	checker()
	{
		std::fill(check_list_, check_list_+BASE, false);
	}
	bool operator()(char c)
	{
		return std::isdigit(c) && !check_list_[c-'0'] && (check_list_[c-'0']=true);
	}
};

#endif
