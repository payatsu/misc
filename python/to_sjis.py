#!/usr/bin/python3
# -*- coding: utf-8 -*-

if __name__ == "__main__":
	import sys
	from os.path import splitext
	for in_file in sys.argv[1:]:
		path, ext = splitext(in_file)
		open(path + '_sjis' + ext, 'wb').write(open(in_file).read().encode('sjis'))
