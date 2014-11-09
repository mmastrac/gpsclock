#!/usr/bin/python
"""Display driver tester.

Usage:
  tester.py --help | --port PORT | --list -v

Options:
  -h --help     Show this screen.
  --port PORT   Select the serial port.
  --list        List the serial ports.
  -v            Verbose mode.

"""
import sys
import time
import serial.tools.list_ports
from serial import Serial
from docopt import docopt

def list():
    for x in serial.tools.list_ports.comports():
        print "Port: " + x[0]
        print "Desc: " + x[1]
        print "HWID: " + x[2]
        print ""

def loop(port, verbose):
    print "Initializing..."
    p = Serial(port=port, timeout=0)
    state = -1
    match = "Done."
    while True:
        c = p.read()
        if len(c) == 0:
            if verbose:
                print "-> ~"
            p.write('~')
            time.sleep(0.1)
            continue
        if verbose:
            print "<- " + c
        if c == 'D':
            state = 1
            continue
        if state >= 0:
            if c == match[state]:
                state = state + 1
                if state >= len(match):
                    break
                continue
    print "Initialized."

    while True:
        print ">",
        raw_input()

if __name__ == '__main__':
    arguments = docopt(__doc__, version='Display driver tester v1.0')
    
    if arguments['--list']:
        list()
        sys.exit(1)

    if arguments['--port']:
        loop(arguments['--port'], arguments['-v'])
        sys.exit(0)

    sys.exit(1)