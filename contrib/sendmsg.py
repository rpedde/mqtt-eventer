#!/usr/bin/env python

import argparse
import sys

import paho.mqtt.client as mqtt


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")


def get_parser():
    aparser = argparse.ArgumentParser(description='mqtt sender')

    aparser.add_argument('--host', help='mqtt host',
                         default='localhost')
    aparser.add_argument('--port', help='mqtt port',
                         default=1883, type=int)
    aparser.add_argument('--json', help='format message as json',
                         action='store_false')


def main(rawargs):
    args = get_parser.parse_args(rawargs)

    
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
