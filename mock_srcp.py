#!/usr/bin/python

import socket
import threading
import SocketServer
import time
import sys

MODE_HANDSHAKE = "HANDSHAKE"
MODE_INFO      = "INFO"
MODE_COMMAND   = "COMMAND"

_client_id = 0
_sessions = []
_time = 0
_continue = True

class SessionRequestHandler(SocketServer.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)

    def setup(self):
        self.mode = MODE_HANDSHAKE
        global _sessions
        _sessions.append(self)
        self.session_id = len(_sessions)
        print "Add session", self.session_id
        self.power = "OFF"

    def reply(self, s):
        global _time
        _time += 1
        s = "%ld %s" % (_time, s)
        print "[%d] <" % self.session_id, s
        self.request.sendall(s + "\n")

    def handle(self):
        global _sessions
        # Note for JMRI I seem to have to disable the header, why??
        header = "SRCP 0.8.4; SRCPOTHER 0.8.3"
        print "[%d] <" % self.session_id, header
        self.request.sendall(header + "\n")

        while True:
            c = self.request.recv(1000).strip()
            print "[%d] >" % self.session_id, c
            
            # Handshake mode
            if c.startswith("SET PROTOCOL SRCP "):
                self.reply("201 OK PROTOCOL SRCP")
            elif c.startswith("SET CONNECTIONMODE SRCP "):
                self.mode = c[ len("SET CONNECTIONMODE SRCP ") : ]
                self.reply("202 OK CONNECTIONMODE")
            elif c == "GO":
                global _client_id
                _client_id += 1
                self.reply("200 OK GO %s" % _client_id)
                if self.mode == MODE_INFO:
                    self.reply("100 INFO 0 DESCRIPTION SESSION SERVER TIME")
                    self.reply("100 INFO 0 TIME 1 1")  # time ratio fx:fy
                    self.reply("100 INFO 0 TIME 12345 23 59 59")  # time julianDay HH MM SS
                    self.reply("100 INFO 1 POWER ON blah blah")
                    self.reply("100 INFO 6 DESCRIPTION GA DESCRIPTION")
                    self.reply("100 INFO 6 GA 1 0 0")    # turnout #0, port 0 value 0
                    self.reply("100 INFO 6 GA 1 1 0")    # turnout #0, port 1 value 0
                    self.reply("100 INFO 8 DESCRIPTION FB DESCRIPTION")
                    self.reply("100 INFO 8 FB 1 1")      # point-detector #1, value 1
                    self.reply("100 INFO 8 FB 2 0")      # point-detector #2, value 0
                    for session in _sessions:
                        if session is not None:
                            self.reply("101 INFO 0 SESSION %d %s" % (session.session_id, session.mode))
            elif c == "GET 6 DESCRIPTION":
                self.reply("100 INFO 6 GA 1 0 0")    # turnout #0, port 0 value 0
                self.reply("100 INFO 6 GA 1 1 0")    # turnout #0, port 1 value 0
            elif c == "GET 8 DESCRIPTION":
                self.reply("100 INFO 8 FB 1 1")      # point-detector #1, value 1
                self.reply("100 INFO 8 FB 2 0")      # point-detector #2, value 0
            elif c == "SET 1 POWER ON":
                self.reply("200 OK")
            elif c == "GET 1 POWER":
                self.reply("100 INFO " + self.power)
            elif c.startswith("INIT 1 POWER "):
                self.power = c[ len("INIT 1 POWER ") : ]
                self.reply("101 INFO " + self.power)
            elif c.startswith("TERM 0 SESSION"):
                # client asked to close this connection
                self.reply("200 OK")
                return
            elif c.startswith("TERM 0 SERVER"):
                # client asked to close this server
                self.reply("200 OK")
                global _continue
                _continue = False
                global server
                server.shutdown()
                return
            else:
                self.reply("200 OK")
                #--reply("400 What?")

    def finish(self):
        print "Remove session", self.session_id
        _sessions[self.session_id - 1] = None


class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass

server = ThreadedTCPServer( ("localhost", 4303), SessionRequestHandler)
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
