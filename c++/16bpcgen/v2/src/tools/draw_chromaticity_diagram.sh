#!/bin/sh -e

tmp_exec=`mktemp`
tmp_data=`mktemp tmp.XXXXXX.txt`
tmp_gnuplot=`mktemp tmp.XXXXXX.gp`
trap 'rm -f ${tmp_exec} ${tmp_data} ${tmp_gnuplot}' 0 INT QUIT

g++ -x c++ -std=c++03 -O3 -Wall -Wextra -o ${tmp_exec} - <<- 'EOF'
	#include <iomanip>
	#include <iostream>

	int main(void)
	{
		const double max_x = 0.73469;
		const double min_x = 0.17310;
		const double max_y = 0.26531;
		const double min_y = 0.00477;
		for(double x = min_x + 0.01; x < max_x; x += 0.01){
			const double y = ((max_x - x)*min_y + (x - min_x)*max_y)/(max_x - min_x);
			std::cout << x << ' '
					  << y << ' '
					  << 1.0 - x - y
					  << std::endl;
		}
		return 0;
	}
EOF
${tmp_exec} > ${tmp_data}

sed -e "s%datafile_placeholder%${tmp_data}%" << 'EOF' > ${tmp_gnuplot}
	set terminal pngcairo enhanced size 1280, 960 font ", 14"
	set output './img/color_matching_functions.png'
	set xlabel 'wave length, {/Symbol l}[nm]'
	set ylabel 'tristimulus values'
	set xrange [300:800]
	set yrange [0:2]
	set grid
	plot './src/assets/color_matching_functions.txt' using 1:2 with lines linewidth 2 title 'color matching function, x({/Symbol l})' \
	   , './src/assets/color_matching_functions.txt' using 1:3 with lines linewidth 2 title 'color matching function, y({/Symbol l})' \
	   , './src/assets/color_matching_functions.txt' using 1:4 with lines linewidth 2 title 'color matching function, z({/Symbol l})'

	set terminal qt enhanced size 1280, 960 font ", 14"
	set xlabel 'x'
	set ylabel 'y'
	set zlabel 'z'
	set xrange [0:1]
	set yrange [0:1]
	set zrange [0:1]
	set grid xtics ytics ztics
	set ticslevel 0
	set border 4095
	set view 60, 120
	max(a, b) = a < b ? b : a;
	min(a, b) = a < b ? a : b;
	Y = 1.0
	X(x, y) = x*Y/y
	Z(z, y) = z*Y/y
	R(X, Y, Z) = int(max(min(( 0.418452000*X - 0.15865200*Y - 0.0828342*Z + 0.09397300) * 255 / 0.441742, 255), 0))
	G(X, Y, Z) = int(max(min((-0.091164200*X + 0.25242400*Y + 0.0157058*Z + 0.00142583) * 255 / 0.216629, 255), 0))
	B(X, Y, Z) = int(max(min(( 0.000920718*X - 0.00254938*Y + 0.1785950*Z + 0.00138149) * 255 / 0.320050, 255), 0))
	color(X, Y, Z) = R(X, Y, Z)*65536 + G(X, Y, Z)*256 + B(X, Y, Z)
	splot './src/assets/color_matching_functions.txt' using 5:6:7:(color($2, $3, $4)) with lines linewidth 25 linecolor rgbcolor variable title '' \
		, '' using (1.0/3.0):(1.0/3.0):(1.0/3.0):($5 - 1.0/3.0):($6 - 1.0/3.0):($7 - 1.0/3.0):(color($2, $3, $4)) with vectors nohead linewidth 25 linecolor rgbcolor variable title '' \
		, 'datafile_placeholder' using 1:2:3:(color(X($1, $2), Y, Z($3, $2))) with lines linewidth 25 linecolor rgbcolor variable title '' \
		, '' using (1.0/3.0):(1.0/3.0):(1.0/3.0):($1 - 1.0/3.0):($2 - 1.0/3.0):($3 - 1.0/3.0):(color(X($1, $2), Y, Z($3, $2))) with vectors nohead linewidth 25 linecolor rgbcolor variable title '' \
		, 1-x-y with lines linecolor rgbcolor 'black'
	pause -1
EOF
mkdir -p ./img
gnuplot ${tmp_gnuplot}
