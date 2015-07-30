#!/usr/bin/python

import socket
import threading
import SocketServer
import time
import sys

SRCP_MODE_HANDSHAKE = "HANDSHAKE"
SRCP_MODE_INFO      = "INFO"
SRCP_MODE_COMMAND   = "COMMAND"

_nce_server = None
_srcp_server = None
_srcp_client_id = 0
_srcp_sessions = []
_time = 0
_continue = True

def toggle_turnout(index):
    try:
        print "Toggle turnout", index
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect( ("192.168.1.140", 8080) )
        try:
            sock.sendall("@%02d\n" % (index+1))
        finally:
            sock.close()
    except:
        print "Toggle turnout FAILED"


class NceRequestHandler(SocketServer.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)

    def setup(self):
        self.buf_len = 5
        self.buf = bytearray(self.buf_len)
        self.last_sensors = 0
        print "[NCE] Start handler"

    def handle(self):
        buf = self.buf
        conn = self.request
        n = conn.recv_into(buf, self.buf_len)
        if n == 0:
            return
        c = buf[0]
        print "[NCE] >", n, "%02x|%c" % (c, c > 32 and c or " "), \
            "%02x" % buf[1], "%02x" % buf[2], "%02x" % buf[3], "%02x" % buf[4] 
        
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

    def finish(self):
        print "[NCE] End handler"

        
class SrcpRequestHandler(SocketServer.BaseRequestHandler):
    def __init__(self, request, client_address, server):
        SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)

    def setup(self):
        self.mode = SRCP_MODE_HANDSHAKE
        global _srcp_sessions
        _srcp_sessions.append(self)
        self.session_id = len(_srcp_sessions)
        print "[SRCP] Add session", self.session_id
        self.power = "OFF"

    def reply(self, s):
        global _time
        _time += 1
        s = "%ld %s" % (_time, s)
        print "[SRCP %d] <" % self.session_id, s
        self.request.sendall(s + "\n")

    def handle(self):
        global _srcp_sessions
        # Note for JMRI I seem to have to disable the header, why??
        header = "SRCP 0.8.4; SRCPOTHER 0.8.3"
        print "[SRCP %d] <" % self.session_id, header
        self.request.sendall(header + "\n")

        while True:
            try:
                c = self.request.recv(1000).strip()
                print "[SRCP %d] >" % self.session_id, c
            except socket.timeout:
                if self.mode == SRCP_MODE_INFO:
                    # send sensor update
                    print "[SRCP %d] < Update sensors" % self.session_id
                    sensors = int(time.time()) & 0x0F
                    for i in [1, 2, 3, 4]:
                        self.reply("100 INFO 8 FB %d %d" % (i, (sensors >> (i-1)) & 0x1))
                    continue
                    
            
            # Handshake mode
            if c.startswith("SET PROTOCOL SRCP "):
                self.reply("201 OK PROTOCOL SRCP")
            
            elif c.startswith("SET CONNECTIONMODE SRCP "):
                self.mode = c[ len("SET CONNECTIONMODE SRCP ") : ]
                self.reply("202 OK CONNECTIONMODE")
                
                if self.mode == SRCP_MODE_INFO:
                    self.request.settimeout(1)
            
            elif c == "GO":
                global _srcp_client_id
                _srcp_client_id += 1
                self.reply("200 OK GO %s" % _srcp_client_id)
                if self.mode == SRCP_MODE_INFO:
                    self.reply("100 INFO 0 DESCRIPTION SESSION SERVER TIME")
                    self.reply("100 INFO 0 TIME 1 1")  # time ratio fx:fy
                    self.reply("100 INFO 0 TIME 12345 23 59 59")  # time julianDay HH MM SS
                    self.reply("100 INFO 1 POWER ON blah blah")
                    self.reply("100 INFO 7 DESCRIPTION GA DESCRIPTION")
                    self.reply("100 INFO 7 GA 1 0 0")    # turnout #1, port 0 value 0
                    self.reply("100 INFO 7 GA 1 1 0")    # turnout #1, port 1 value 0
                    self.reply("100 INFO 8 DESCRIPTION FB DESCRIPTION")
                    self.reply("100 INFO 8 FB 1 1")      # point-detector #1, value 1
                    self.reply("100 INFO 8 FB 2 0")      # point-detector #2, value 0
                    for session in _srcp_sessions:
                        if session is not None:
                            self.reply("101 INFO 0 SESSION %d %s" % (session.session_id, session.mode))
                        
            elif c.startswith("SET 7 GA 1 "):  # 6=DCC bus, 7=rocrail specific bus
                value = c[11]
                print value
                if value == '0' or value == '1':
                    toggle_turnout(int(value))
                self.reply("200 OK")
            
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
                global _srcp_server
                _srcp_server.shutdown()
                return
            
            else:
                self.reply("200 OK")
                #--reply("400 What?")

    def finish(self):
        print "[SRCP] Remove session", self.session_id
        _srcp_sessions[self.session_id - 1] = None


class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass

def main():
    global _srcp_server, _nce_server
    _srcp_server = ThreadedTCPServer( ("localhost", 4303), SrcpRequestHandler)
    _srcp_server_thread = threading.Thread( target=_srcp_server.serve_forever )
    _srcp_server_thread.daemon = True
    _srcp_server_thread.start()

    _nce_server = ThreadedTCPServer( ("localhost", 8080), NceRequestHandler)
    _nce_server_thread = threading.Thread( target=_nce_server.serve_forever )
    _nce_server_thread.daemon = True
    _nce_server_thread.start()

    _c = 0
    while _continue:
        if _c == 0:
            print "Server loop running in thread:", _srcp_server_thread.name, _nce_server_thread.name
        _c += 1
        if _c == 60: _c = 0
        time.sleep(1) # seconds

    _srcp_server.shutdown()
    _nce_server.shutdown()

if __name__ == "__main__":
    main()
