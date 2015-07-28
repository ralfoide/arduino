#!/usr/bin/python

import translate
import unittest


class MockRequest():
    def __init__(self):
        self._send_str = None
        self._timeout = None
        
    def send(self, str):
        self.sendall(str)
        
    def sendall(self, str):
        self._send_str = str
    
    def recv_into(self, buf, buf_len):
        return 0
    
    def settimeout(self, to):
        self._timeout = to
    

class NceTestCase(unittest.TestCase):
    def testNce(self):
        request = MockRequest()
        h = translate.NceRequestHandler(request, client_address=None, server=None)
        h.setup()
        h.request = request
        h.handle()
        self.assertEqual(None, request._timeout)
        self.assertEqual("", request._send_str)



if __name__ == "__main__":
    unittest.main()
