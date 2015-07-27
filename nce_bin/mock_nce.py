#!/usr/bin/python

import socket
import time
import sys


HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 8080
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr

buf_len = 5
buf = bytearray(buf_len)

last_sensors = 0

while True:
    n = conn.recv_into(buf, buf_len)
    if n == 0:
        continue
    c = buf[0]
    print ">", n, "%02x|%c" % (c, c > 32 and c or " "), "%02x" % buf[1], "%02x" % buf[2], "%02x" % buf[3], "%02x" % buf[4] 
    
    if n == 1 and c == 0xAA:
        # reply with version 6.3.8
        print "Version"
        conn.send("060308".decode("hex"))
    
    elif n == 2 and c == 0x8A:
        # AIU polling, returns 4 bytes (sensor BE, mask BE, 14 bits max)
        print "Poll AIU", buf[1]
        sensors = int(time.time() / 2) & 0x0FFFF
        mask = sensors ^ last_sensors
        last_sensors = sensors
        conn.send( ("%04x%04x" % (sensors, mask)).decode("hex"))
    
    elif n == 4 and c == 0xAD:
        # triggering turnouts
        print "Trigger Acc"
        addr = (buf[1] << 8) + buf[2]
        op = buf[3]
        if op == 3:
            print "ACC normal direction/ON", addr
        elif op == 4:
            print "ACC reverse direction/OFF", addr
        conn.send("!")
    
    elif n == 3 and c == 0x9D:
        # read one byte from RAM
        addr = (buf[1] << 8) + buf[2]
        print "Read RAM at", addr, "(0x%04x)" % addr
        conn.send("00".decode("hex"))
    
    elif n == 3 and c == 0x8F:
        # read 16-bytes from the address specified
        addr = (buf[1] << 8) + buf[2]
        print "Read 16-bytes at", addr, "(0x%04x)" % addr

        turnout = int(time.time()) & 0x0FF
        conn.send( ("%02x" % turnout + "0"*30).decode("hex"))

    elif n == 2 and c == ord("T"):
        # Test
        # reply with version 1.2.3
        print "Test"
        conn.send("630")
    else:
        conn.send("!")

conn.close()

