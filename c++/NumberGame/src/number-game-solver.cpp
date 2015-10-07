/**
 * @file number-game-solver.cpp
 * @details 数当てゲームに回答してくるプログラム．
 */
#include<ctime>
#include<iostream>
#include<fstream>
#include<string>
#include<set>
#include<vector>
#include<algorithm>
#include<random>
#include"common.hpp"
template<base_t range>
std::set<number>& all_combination(base_t i, std::set<number>& numbers, const std::vector<base_t>& seq)
{
	for( ; i<range ; i++){
		auto tmp = seq;
		tmp.push_back(i);
		all_combination<range+1>(i+1, numbers, tmp);
	}
	return numbers;
}
template<>
std::set<number>& all_combination<BASE>(base_t i, std::set<number>& numbers, const std::vector<base_t>& seq)
{
	for( ; i<BASE ; i++){
		auto tmp = seq;
		tmp.push_back(i);
		do{
			number n;
			std::copy(tmp.begin(), tmp.end(), n.number_);
			numbers.insert(n);
		}while(std::next_permutation(tmp.begin(), tmp.end()));
	}
	return numbers;
}
std::set<number> all_combination()
{
	std::set<number> numbers;
	all_combination<BASE-DIGIT+1>(0, numbers, {});
	return numbers;
}
int main()
{
	std::ios::sync_with_stdio(false);
	std::mt19937 engine{static_cast<std::mt19937::result_type>(std::time(nullptr))};
	auto numbers = all_combination();
	std::ofstream ofs("./tmp.log");
	while(!numbers.empty()){
		std::cout << *numbers.begin() << std::endl;
		std::string response;
		std::getline(std::cin, response);
		ofs << response << std::endl;
		numbers.erase(numbers.begin());
	}
	return 0;
}
