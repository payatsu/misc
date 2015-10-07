#include<cstdio>
#include<cassert>
#include<unistd.h>

#include<string>
#include<ostream>

class style{
	int style_;
public:
	enum{
		bold = 1,
		under = 2,
		reverse = 4,
	};
	explicit style(int s=0): style_(s){}
	operator int()const{return style_;}
};
class fg{
	int fg_;
public:
	explicit fg(int f=39): fg_(f){}
	operator int()const{return fg_;}
};
class bg{
	int bg_;
public:
	explicit bg(int b=49): bg_(b){}
	operator int()const{return bg_;}
};
template<typename T>
class attr{
	const T& t_;
	style style_;
	fg fg_;
	bg bg_;
public:
	attr(const T& t): t_(t), style_(), fg_(), bg_(){}
	attr& operator()(style s){style_ = s; return *this;}
	attr& operator()(fg f){fg_ = f; return *this;}
	attr& operator()(bg b){bg_ = b; return *this;}
	template<typename CharT, typename Traits>
	std::basic_ostream<CharT, Traits>& print(std::basic_ostream<CharT, Traits>& bos)const
	{
		assert(30<=fg_ && fg_<=39 || 90<=fg_ && fg_<=99);
		assert(40<=bg_ && bg_<=49 || 100<=bg_ && bg_<=109);
		bos << "[33";
		if(style_ & style::bold){
			bos << 1 << ';';
		}
		if(style_ & style::under){
			bos << 4 << ';';
		}
		if(style_ & style::reverse){
			bos << 7 << ';';
		}
		bos << fg_ << ';' << bg_ << 'm' << t_ << "[K[m[K";
		return bos;
	}
};
template<typename CharT, typename Traits, typename T>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& bos, const attr<T>& c)
{
	return c.print(bos);
}
template<typename T>
attr<T> col(const T& t)
{
	return attr<T>(t);
}

FILE* print(FILE* fp, const char* const str, style s, int fg, int bg, bool force = true)
{
	assert(30<=fg && fg<=39 || 90<=fg && fg<=99);
	assert(40<=bg && bg<=49 || 100<=bg && bg<=109);
	if(!force && !isatty(fileno(fp))){
		fprintf(fp, "%s", str);
		return fp;
	}
	fprintf(fp, "[33");
	if(s & style::bold){
		fprintf(fp, "%d;", 1);
	}
	if(s & style::under){
		fprintf(fp, "%d;", 4);
	}
	if(s & style::reverse){
		fprintf(fp, "%d;", 7);
	}
	fprintf(fp, "%d;%dm%s[k[m[k", fg, bg, str);
	return fp;
}

#include<iostream>
int main()
{
	const char* const hello = "Hello ";
	for(int b=40 ; b<50 ; b++){
		for(int f=30 ; f<40 ; f++){
			for(int s=1 ; s<8 ; s++){
				std::cout << col(hello)(style(s))(fg(f))(bg(b));
				std::cout << col(hello)(style(s))(fg(f+60))(bg(b));
				// print(stdout, hello, style(s), f, b);
				// print(stdout, hello, style(s), f+60, b);
			}
			for(int s=1 ; s<8 ; s++){
				std::cout << col(hello)(style(s))(fg(f))(bg(b+60));
				std::cout << col(hello)(style(s))(fg(f+60))(bg(b+60));
				// print(stdout, hello, style(s), f, b+60);
				// print(stdout, hello, style(s), f+60, b+60);
			}
			putchar('\n');
		}
	}
	return 0;
}
