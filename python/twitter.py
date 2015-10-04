#!/usr/bin/python3
# -*- coding: utf-8 -*-

import argparse, datetime, requests_oauthlib, sys

def tweet(consumer_key, consumer_secret, access_token, access_token_secret, twt):
	twitter = requests_oauthlib.OAuth1Session(consumer_key, consumer_secret, access_token, access_token_secret)
	req = twitter.post("https://api.twitter.com/1.1/statuses/update.json", params = {"status": twt})
	if req.status_code == 200:
		print("OK")
	else:
		print("Error: {}".format(req.status_code))

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--consumer-key')
	parser.add_argument('--consumer-secret')
	parser.add_argument('--access-token')
	parser.add_argument('--access-token-secret')
	parser.add_argument('--tweet')
	args = parser.parse_args()
	tweet(args.consumer_key, args.consumer_secret, args.access_token, args.access_token_secret, args.tweet)
