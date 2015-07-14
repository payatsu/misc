#!/usr/bin/python3

import argparse
import datetime

parser = argparse.ArgumentParser(description = 'Calculate your age.')
parser.add_argument('-y', '--year', default = 1964, type = int)
parser.add_argument('-m', '--month', default = 9, type = int)
parser.add_argument('-d', '--day', default = 25, type = int)
args = parser.parse_args()

birthday = datetime.date(args.year, args.month, args.day)
today = datetime.date.today()

delta = today - birthday

years = delta.days // 365
months = 0
days = delta.days % 365
if 17 < years:
	months = (years - 17) * 12
	years = 17

print('You are {years} years, {months} months, {days} days old.'.format(years = years, months = months, days = days))
