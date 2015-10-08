/**
 * @file number-game-questioner.cpp
 * @details  数当てゲームを出題してくるプログラム．
 */

#include<cstring>
#include<ctime>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<random>
#include<memory>
#include<readline/readline.h>
#include<readline/history.h>
#include"common.hpp"

inline bool validate_input(std::shared_ptr<const char> input)
{
	return input == nullptr || !std::strcmp(input.get(), "") ||
		(std::strlen(input.get()) == DIGIT &&
		std::count_if(input.get(), input.get()+DIGIT, checker()) == DIGIT);
}

inline bool notify_input_error()
{
	std::cerr << INVALID_INPUT << std::endl;
	return true;
}

inline number to_number(std::shared_ptr<const char> input)
{
	number n;
	std::transform(input.get(), input.get()+DIGIT, n.number_, [](char c){return c - '0';});
	return n;
}

inline bool judge(const number& hidden, const number& query)
{
	digit_t eat = 0;
	digit_t bite = 0;
	for(digit_t i=0 ; i<DIGIT ; i++){
		if(query.number_[i]==hidden.number_[i]){
			eat++;
		}
		if(std::find(hidden.number_, hidden.number_+DIGIT, query.number_[i]) != hidden.number_+DIGIT){
			bite++;
		}
	}
	std::cout << eat << " eats, " << bite-eat << " bites." << std::endl;
	return eat == DIGIT;
}

inline bool notify_congratulations()
{
	std::cout << CONGRATULATIONS << std::endl;
	return false;
}

int main()
{
	std::ios::sync_with_stdio(false);
	std::mt19937 engine{static_cast<std::mt19937::result_type>(std::time(nullptr))};
	number hidden;
	std::generate(hidden.number_, hidden.number_+DIGIT, generator(engine));
	std::shared_ptr<const char> line;
	int turns = 0;
	do{
		do{
			turns++;
			line.reset(readline(PROMPT), std::free);
			add_history(line.get());
		}while(!validate_input(line) && notify_input_error());
		if(line == nullptr){
			std::cout << BYE << std::endl;
			break;
		}else if(!std::strcmp(line.get(), "")){
			continue;
		}
	}while(!judge(hidden, to_number(line)) || notify_congratulations());
	clear_history();
	std::cout << turns << " turns." << std::endl;
	return 0;
}
