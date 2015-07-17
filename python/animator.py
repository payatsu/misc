#!/usr/bin/python3
# -*- coding: utf-8 -*-

'''
Python script for generating an animation.
@note Run this script with -H option for more details. This shows you help.
'''

import argparse
import distutils.spawn
import glob
import os
import subprocess
import sys
import math

# configuration parameters

trace_prefix = 'node_'
trace_suffix = 'trc'

position_prefix = 'time_'
position_suffix = 'pos'

time_fmt = '9.2f'
time_filled_fmt = '0' + time_fmt

x_label = 'x [m]'
y_label = 'y [m]'

time_unit = '[sec]'

gnuplot_conf = 'gnuplot.conf'
color_col = 6

# default parameters, and its values
img_terminals = {'png': 'pngcairo', 'jpeg': 'jpeg', 'jpg': 'jpeg', 'gif': 'gif'}
output_animation = 'animation.mp4'

def show_help(file = sys.stdout):
	text = \
	'''
                    << ヘルプ >>
[名前]
	{prog_name} - アニメーション動画を生成する．

[書式]
	{prog_name} [-a] [-c] [-b begin] [-e end] [-i interval] [-t directory]

[説明]
	テキストログをもとに，ノードが動くアニメーション動画を生成する
	スクリプト．
	** 無保証 **

[オプション]
	-a
		連続する静止画からの動画生成を有効化
	-b value
		対象期間の開始時刻を value に設定
	-c
		ポジションファイルからの静止画生成を有効化
	-e value
		対象区間の終了時刻を value に設定
	-f format
		生成する静止画の形式を format に指定
		format は gif, jpeg, jpg, png の中からどれかを指定
	-h value
		生成する静止画の高さサイズを value に設定
	-H
		このヘルプを表示する
	-i value
		生成する静止画の時間ステップ幅を value 指定
		ポジションファイル群と整合性を取る必要あり
	-r value
		生成する動画のフレームレートを value に指定
	-s
		トレースファイルからのポジションファイル生成を有効化
	-t value
		ターゲットディレクトリを value に指定
	-w value
		生成する静止画の幅サイズを value に指定

[用語]
	トレースファイル
		ファイル名が {trace_prefix}nnnn.{trace_suffix} となっている
		テキストファイル．ここで， nnnn はノード番号を表す．
		トレースファイルの構文は， 1 カラム目が時刻， 2 カラム目以降が
		その時刻におけるそのノードに関する付加的な情報となっている．
		これを必要な行数分だけ列挙することでトレースファイルが構成される．
		1 カラム目の書式は，time_fmt で指定した printf 書式(%{time_fmt})で
		表現されていなければならない．

		例：テキストファイル {trace_prefix}001234.{trace_suffix} の
		の中身が，
		{t1:{time_fmt}} 3.14 2.718 1.414 1.732 1
		{t2:{time_fmt}} 1.11 2.222 3.333 4.444 1
		となっている場合，これはノード番号 1234 のノードのトレースファイル
		であり，その一行目は時刻 567 におけるノード 1234 の情報を表し，
		その二行目は時刻 568 におけるノード 1234 の情報を表している．
		この例では，3.14 や 1.11 が x 座標， 2.718 や 2.222 が y 座標，
		1.414 や 3.333 が x 方向の速度， 1.732 や 4.444 が y 方向の速度，
		最後のカラムが持っている情報数を表すことにしているが，実際に
		2 カラム目以降に何を意味づけするかは使用者次第で決めてもよい．

		トレースファイルがあればポジションファイルを生成できるが，
		ポジションファイルを直接用意した方がこのツール的には効率が良い．

	ポジションファイル
		ファイル名が {position_prefix}tttt.{position_suffix} となっている
		テキストファイル．ここで，ttttは時刻を表し，time_filled_fmt で指定した
		printf 書式(%{time_filled_fmt})で表現されていなければならない．
		ポジションファイルの構文は， 1 カラム目がノード番号， 2 カラム目以降
		がその時刻におけるそのノードに関する付加的な情報となっている．
		これを必要な行数分だけ列挙することでポジションファイルが構成される．

		例：テキストファイル {position_prefix}{t1:{time_filled_fmt}}.{position_suffix} の
		中身が，
		1234 3.14 2.718 1.414 1.732 1
		1235 0.301 0.477 0.602 0.698 2
		となっている場合，これは時刻 567 のポジションファイルであり，その
		一行目は時刻 567 におけるノード番号 1234 のノードの情報を表し，
		その二行目は時刻 567 におけるノード番号 1235 のノードの情報を
		表している．この例では， 3.14 や 0.301 が x 座標， 2.718 や 0.477 が
		y 座標， 1.414 や 0.602 が x 方向の速度， 1.732 や 0.698 が y 方向の
		速度，最後のカラムが持っている情報数を表すことにしているが，実際に
		2 カラム目以降に何を意味付けするかは使用者次第で決めてもよい．

	{gnuplot_conf}
		ポジションファイルから静止画を生成する際に読み込まれる gnuplot
		用設定ファイル．必ず用意しなければならないわけではなく，用意した
		場合のみ読み込まれ，その設定内容が反映される．実際のところ，
		set xrange や set yrange を指定しないと動画が揺れることから，
		set xrange や set yrange を指定した {gnuplot_conf} を作成した
		方が良い．

		例えば以下のような内容で作成する．
		set size square
		set grid lt 1 lc rgb 'black'
		set xrange [-100:100]
		set yrange [-100:100]
		set style line 1 lc rgb 'web-green'
		set style line 2 lc rgb 'red'

	ターゲットディレクトリ
		トレースファイルやポジションファイル， {gnuplot_conf} が置かれているディレクトリ．
		静止画や動画もこのディレクトリに出力される．

[動画生成の仕組み]
	動画は次の手順に沿って生成される．
	0. トレースファイルからポジションファイルを生成する（ -s オプション）．
	1. ポジションファイルから静止画を生成する（ -c オプション）．
	2. 1. で生成した静止画を連結して動画を生成（ -a オプション）．

[使い方]
	1. ポジションファイルを用意する．
	2. このスクリプトを実行する．

[使用例]
	1) log ディレクトリにあるポジションファイルを使って時刻 0 から 600
	   の間の動画を生成．時間間隔は 1 ．
		$ {prog_name} -ac -b 0 -e 600 -t log

	2) ポジションファイルのサンプルのアニメ化．
		$ {prog_name} -ac -b 0 -e 600 -S

[動作要件]
	このスクリプトの動作には以下のツール（コマンド）が必要
	* gnuplot
	* avconv または ffmpeg
	これらのインストールは例えば次のようにして行える．
		$ sudo apt-get install gnuplot avconv
	'''.format(
		prog_name = os.path.basename(sys.argv[0]),
		trace_prefix = trace_prefix,
		trace_suffix = trace_suffix,
		t1 = 567,
		t2 = 568,
		position_prefix = position_prefix,
		position_suffix = position_suffix,
		time_fmt = time_fmt,
		time_filled_fmt = time_filled_fmt,
		gnuplot_conf = gnuplot_conf
	)
	print(text)

def frange(x, y, jump):
	while x < y:
		yield x
		x += jump

def startup():
	parser = argparse.ArgumentParser(description = 'Generate an animation.', add_help = False)

	parser.add_argument('-a', action = 'store_true')
	parser.add_argument('-b', default = 0, type = float)
	parser.add_argument('-c', action = 'store_true')
	parser.add_argument('-C', action = 'store_true')
	parser.add_argument('-e', default = 1000, type = float)
	parser.add_argument('-f', default = 'png', choices = ['png', 'jpg', 'jpeg', 'gif'])
	parser.add_argument('-h', default = 600, type = int)
	parser.add_argument('-H', action = 'store_true')
	parser.add_argument('-i', default = 1, type = float)
	parser.add_argument('-r', default = 20, type = int)
	parser.add_argument('-s', action = 'store_true')
	parser.add_argument('-S', action = 'store_true')
	parser.add_argument('-t', default = '.')
	parser.add_argument('-w', default = 800, type = int)
	parser.add_argument('-x', default = 2, type = int)
	parser.add_argument('-y', default = 3, type = int)

	return parser.parse_args(sys.argv[1:])

def generate_sample():
	def hypotrochoid(theta):
		rc = 50.0
		rm = 30.0
		rd = 50.0
		return ((rc-rm)*math.cos(theta) + rd*math.cos((rc-rm)*theta/rm),
				(rc-rm)*math.sin(theta) - rd*math.sin((rc-rm)*theta/rm))

	def epitrochoid(theta):
		rc = 30.0
		rm = 20.0
		rd = 5.0
		return ((rc+rm)*math.cos(theta) - rd*math.cos((rc+rm)*theta/rm),
				(rc+rm)*math.sin(theta) - rd*math.sin((rc+rm)*theta/rm))

	def generate_gnuplot_conf():
		with open('gnuplot.conf', 'w') as conf:
			conf.write(
				'''
				set size square
				set grid lt 1 lc rgb 'black'
				set xrange [-100:100]
				set yrange [-100:100]
				set style line 1 lc rgb 'web-green'
				set style line 2 lc rgb 'red'
				''')

	def main():
		print('generating sample position files...', end = '', flush = True)

		generate_gnuplot_conf()
		freq = 2.0 ** -7.0
		dt = 1.0
		limit = 600.0

		for t in frange(0.0, limit, dt):
			with open(('time_{:{}}.pos').format(t, time_filled_fmt), 'w') as pos:
				s = 0.0
				for s in frange(0.0, t, dt):
					pos.write('0 {0[0]} {0[1]} 0 0 0\n'.format(hypotrochoid(2*math.pi*freq*s)))
				pos.write('0 {0[0]} {0[1]} 0 0 1\n'.format(hypotrochoid(2*math.pi*freq*s)))
				pos.write('1 {0[0]} {0[1]} 0 0 2\n'.format(epitrochoid(2*math.pi*freq*t)))
		print(' done.')

	main()

def generate_position_files(args):
	print('generating position files...', end = '', flush = True)

	trace_files = glob.glob(args.t + os.sep + trace_prefix + '*.' + trace_suffix)
	if not trace_files:
		print('error: no trace files', file = sys.stderr)
		show_help(sys.stderr)
		exit(1)

	raise NotImplemented

	print(' done.')

def generate_snapshot_images(args):
	load_file = 'load \'' + args.t + os.sep + gnuplot_conf + '\'' if os.path.isfile(args.t + os.sep + gnuplot_conf) else ''

	generating_images = '\rgenerating snapshot images...'
	for time in frange(args.b, args.e, args.i):
		print('{} {} of [begin: {} end: {}]'.format(generating_images, time, args.b, args.e), end = '', flush = True)
		time_filled = ('{:{}}').format(time, time_filled_fmt)
		position_file = args.t + os.sep + position_prefix + time_filled + '.' + position_suffix
		os.path.isfile(position_file) or open(position_file, 'w').close()

		pipe = subprocess.Popen('gnuplot', stdin = subprocess.PIPE)
		pipe.communicate(
			'''
			set terminal {} enhanced size {:d}, {:d} font 'LiberationSans-Regular.ttf, 16'
			set output '{}'
			set xlabel '{}'
			set ylabel '{}'
			set label 1 'time = {:{}}{}' at graph 0.05, 0.95 left
			set key box
			{}
			plot '{}' u {:d}:{:d}:{:d} w p pt 7 ps 2 lc variable t 'nodes'
			'''.format(img_terminals[args.f],
					   args.w, args.h,
					   args.t + os.sep + position_prefix + time_filled + '.' + args.f,
					   x_label, y_label, time, time_fmt, time_unit,
					   load_file, position_file, args.x, args.y, color_col).encode('utf-8'))
		pipe.wait()

	print(generating_images, 'done.                                          ')

def generate_animation(args):
	print('generating an animation video...', end = '', flush = True)

	symlink_fmt = args.t + os.sep + 'snapshot-%05d'
	for img in glob.glob(args.t + os.sep + 'snapshot-*.' + args.f):
		os.remove(img)

	if os.name == 'posix' or os.name == 'nt':
		symlink_fmt_py = symlink_fmt.replace('%', '{:') + '}.' + args.f
		for (offset, item) in enumerate(sorted(glob.glob(args.t + os.sep + position_prefix + '*.' + args.f))):
			if args.b <= float(item.lstrip(args.t + os.sep + position_prefix).rstrip('.' + args.f)) < args.e:
				os.symlink(item, symlink_fmt_py.format(offset))
	else:
		raise 'Unknown OS'

	ffmpeg_or_avconv = distutils.spawn.find_executable('ffmpeg') or distutils.spawn.find_executable('avconv', path = '/usr/bin/')
	subprocess.check_call([ffmpeg_or_avconv, '-loglevel', 'error', '-y', '-i', symlink_fmt + '.' + args.f, '-r', str(args.r), args.t + os.sep + output_animation])

	print(' done.\n'
		  'generated -> \'' + output_animation + '\'')

def cleanup():
	for f in \
		glob.glob(args.t + os.sep + position_prefix + '*.' + position_suffix) \
		+ glob.glob(args.t + os.sep + position_prefix + '*.' + args.f) \
		+ glob.glob(args.t + os.sep + 'snapshot-*.' + args.f):
		os.remove(f)

	os.path.isfile(args.t + os.sep + gnuplot_conf) and os.remove(args.t + os.sep + gnuplot_conf)
	os.path.isfile(args.t + os.sep + output_animation) and os.remove(args.t + os.sep + output_animation)

if __name__ == '__main__':
	args = startup()
	args.H and (show_help() or exit(0))
	args.C and cleanup()
	args.S and generate_sample()
	args.s and generate_position_files(args)
	args.c and generate_snapshot_images(args)
	args.a and generate_animation(args)
