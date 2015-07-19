#!/usr/bin/python

from socket import *

s = socket(AF_INET, SOCK_STREAM)
s.bind( ("localhost", 4303) )
s.listen(5)  # accept up to 5 cnx at the same time

MODE_HANDSHAKE = "HANDSHAKE"
MODE_INFO      = "INFO"
MODE_COMMAND   = "COMMAND"

_client_id = 0
_sessions = []
_time = 0

class Session:
    def __init__(self, conn):
        self.conn = conn
        self.conn.send("RalfMockSvr 1; SRCP 0.8.4; SRCOTHER 0.8.3\n")
        self.mode = MODE_HANDSHAKE
        self.session_id = 0
        self.power="OFF"

    def reply(self, s):
        global _time
        _time += 1
        s = "%ld %s" % (_time, s)
        print self.session_id, "<",s
        self.conn.send(s + "\n")

    def handle(self):
        while True:
            c = self.conn.recv(1000)
            c = c.strip()
            print self.session_id, ">",c
            
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
                    global _sessions
                    for session in _sessions:
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
                conn.close()
                return
            else:
                self.reply("200 OK")
                #--reply("400 What?")


while True:
    conn, address = s.accept()
    session = Session(conn)
    _sessions.append(session)
    session.session_id = len(_sessions)
    session.handle()

s.close()
