#!/usr/bin/python

'''
Python script for generating an animation.
@note Run this script with -H option for more details. This shows you help.
'''

import argparse
import glob
import os
import subprocess
import sys
from math import sin, cos, pi

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
animate_flag = False
begin_time = 0
clip_flag = False
end_time = 1000
img_terminal = 'pngcairo'
img_fmt = 'png'
height = 600
interval = 1
frame_rate = 20
position_file_generate_flag = False
generate_sample = False
target_dir = './'
width = 800
x_col = 2
y_col = 3
output_animation = 'animation.mp4'

def show_help(file):
    aaa = \
    '''
                    << ヘルプ >>
[名前]
	{prog_name} - アニメーション動画を生成する．

[書式]
	{prog_name} [-a] [-c] [-b begin] [-e end] [-i interval] [-t directory]

[説明]
	テキストログをもとに，ノードが動くアニメーション動画を生成する
	シェルスクリプト．
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
		1 カラム目の書式は，TIME_FMT で指定した printf 書式(%{time_fmt})で
		表現されていなければならない．

		例：テキストファイル {trace_prefix}001234.{trace_suffix} の
		の中身が，
		`printf %{time_fmt} 567` 3.14 2.718 1.414 1.732 1
		`printf %{time_fmt} 568` 1.11 2.222 3.333 4.444 1
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

		例：テキストファイル {position_prefix}`printf %{time_filled_fmt} 567`.{position_suffix} の
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
        prog_name=os.path.basename(sys.argv[0]),
        trace_prefix=trace_prefix,
        trace_suffix=trace_suffix,
        position_prefix=position_prefix,
        position_suffix=position_suffix,
        time_fmt=time_fmt,
        time_filled_fmt=time_filled_fmt,
        gnuplot_conf=gnuplot_conf)
    print(aaa)

show_help(None)

def frange(x, y, jump):
    while x < y:
        yield x
        x += jump

def startup():
    parser = argparse.ArgumentParser(description = 'options')

    '''
    TODO Under construction...
    '''

def generate_sample():
    def hypotrochoid(theta):
        rc = 50.0
        rm = 30.0
        rd = 50.0
        return ((rc-rm)*cos(theta) + rd*cos((rc-rm)*theta/rm),
                (rc-rm)*sin(theta) - rd*sin((rc-rm)*theta/rm))

    def epitrochoid(theta):
        rc = 30.0
        rm = 20.0
        rd = 5.0
        return ((rc+rm)*cos(theta) - rd*cos((rc+rm)*theta/rm),
                (rc+rm)*sin(theta) - rd*sin((rc+rm)*theta/rm))

    def generate_gnuplot_conf():
        conf = open('gnuplot.conf', 'w')
        conf.write(
            'set size square\n'
            'set grid lt 1 lc rgb "black"\n'
            'set xrange [-100:100]\n'
            'set yrange [-100:100]\n'
            'set style line 1 lc rgb "web-green"\n'
            'set style line 2 lc rgb "red"\n'
        )
        conf.close()

    def main():
        print('generating sample position files...', end='', flush=True)

        freq = 1.0/2.0/2.0/2.0/2.0/2.0/2.0/2.0
        dt = 1.0
        limit = 600.0

        generate_gnuplot_conf

        for t in frange(0.0, limit, dt):
            pos = open('time_{0: 09.2f}.pos'.format(t), 'w')
            s = 0.0
            for s in frange(0.0, t, dt):
                p = hypotrochoid(2*pi*freq*s)
                pos.write('0 {0} {1} 0 0 0\n'.format(p[0], p[1]))
            p = hypotrochoid(2*pi*freq*(s-dt))
            pos.write('0 {0} {1} 0 0 1\n'.format(p[0], p[1]))
            p = epitrochoid(2*pi*freq*t)
            pos.write('1 {0} {1} 0 0 2\n'.format(p[0], p[1]))
            pos.close()

        print(' done.')
        return 0

    main()

def generate_position_files():
    print('generating position files...', end='', flush=True)

    trace_files = glob.glob(target_dir + os.sep + trace_prefix + '*.' + trace_suffix)
    if not trace_files:
        print('error: no trace files', file=sys.stderr)
        show_help(sys.stderr)
        exit(1)

    '''
    TODO Under construction
    '''

    print(' done.')

def generate_snapshot_images():
    gnuplot_conf = target_dir + os.sep + gnuplot_conf
    if os.path.isfile(gnuplot_conf):
        load_file = 'load "' + gnuplot_conf + '"'

    generating_images = '\rgenerating snapshot images...'
    for time in frange(begin_time, end_time, interval):
        print(generating_images, time, 'of [begin:', begin_time, 'end:', end_time, ']', end='', flush=True)
        time_formatted = '{0: ' + time_fmt + '}'.format(time)
        time_filled = '{0: ' + time_filled_fmt + '}'.format(time)
        position_file = target_dir + os.sep + position_prefix + time_filled + '.' + position_suffix
        output_image = target_dir + os.sep + position_prefix + time_filled + '.' + img_fmt
        if not os.path.isfile(position_file):
            open(position_file, 'w').close()

        pipe = subprocess.Popen('gnuplot', stdin=subprocess.PIPE)
        pipe.write(
            'set terminal ' + img_terminal + ' enhanced size ' + width + ', ' + height + ' font "LiberationSans-Regular.ttf, 16"\n'
            'set output "' + output_image + '"\n'
            'set xlabel "' + x_label + '"\n'
            'set ylabel "' + y_label + '"\n'
            'set label 1 "time = ' + time_formatted + time_unit + '" at graph 0.05, 0.95 left\n'
            'set key box\n'
            + load_file + '\n'
            'plot "' + position_file + '" u ' + x_col + ':' + y_col + ':' + color_col + ' w p pt 7 ps 2 lc variable t "nodes"\n'
        )
        pipe.close()

    print(generating_images, 'done.                                          ')

def generate_animation():
    print('generating an animation video...', end='', flush=True)

    symlink_fmt = target_dir + os.sep + 'snapshot-%05d'
    for img in glob.glob(target_dir + os.sep + 'snapshot-*.' + img_fmt):
        os.remove(img)

    # TODO Under construction
    if os.name == 'nt':
        pass
    elif os.name == 'posix':
        pass
    else:
        raise 'Unknown OS'

    '''
    TODO シンボリックリンクの作成。
    '''

    subprocess.check_call(['avconv', '-loglevel', 'error', '-y', '-i', symlink_fmt + '.' + img_fmt, '-r', frame_rate, target_dir + os.sep + output_animation])

    print(' done.')
    print('generated -> "{0}"'.format(output_animation))

if __name__ == '__main__':
    startup()
