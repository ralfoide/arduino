#!/usr/bin/python

import socket
import threading
import SocketServer
import time
import sys

_continue = True

def toggle_turnout(index):
    print "Toggle turnout", index
#    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#    sock.connect( ("192.168.1.140", 8080) )
#    try:
#        sock.sendall("@%02d\n" % (index+1))
#    finally:
#        sock.close()


class SessionRequestHandler(SocketServer.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)

    def setup(self):
        print "Start Request"

    def reply(self, s):
        self.request.sendall(s + "\n")

    def handle(self):
        print type(self.request)
        print dir(self.request)
        
        buf_len = 5
        buf = bytearray(buf_len)
        n = self.request.recv_into(buf, buf_len)
        print ">", n, buf[0], buf[1], buf[2], buf[3], buf[4] 
        
        global _continue
        _continue = False
        #while True:
        #    c = self.request.recv(1000)
        #    print ">", c

    def finish(self):
        print "End Request"



class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass

    
server = ThreadedTCPServer( ("localhost", 8080), SessionRequestHandler)
server_thread = threading.Thread( target=server.serve_forever )
server_thread.daemon = True
server_thread.start()

_c = 0
while _continue:
    if _c == 0:
        print "Server loop running in thread:", server_thread.name
    _c += 1
    if _c == 60: _c = 0
    time.sleep(1) # seconds

server.shutdown()
