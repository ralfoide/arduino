package translate

import (
    "fmt"
    "testing"
    "github.com/stretchr/testify/assert"
)

func TestSrcpSession_ReplyRaw(t *testing.T) {
    assert := assert.New(t)
    c := newMockConn(nil)
    u := NewSrcpSessions()
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}
    
    assert.Equal(42, s.Id)
    assert.Equal(11, s.Time)
    assert.Equal(SRCP_MODE_HANDSHAKE, s.Mode)
    
    s.ReplyRaw("SRCP PROTOCOL")
    assert.Equal(11, s.Time)
    assert.Equal( []byte("SRCP PROTOCOL\n"), c._write)    
}

func TestSrcpSession_Reply(t *testing.T) {
    assert := assert.New(t)
    c := newMockConn(nil)
    u := NewSrcpSessions()
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}
    
    assert.Equal(42, s.Id)
    assert.Equal(11, s.Time)
    assert.Equal(SRCP_MODE_HANDSHAKE, s.Mode)
    
    s.Reply("SRCP PROTOCOL")
    assert.Equal(12, s.Time)
    assert.Equal( []byte("12 SRCP PROTOCOL\n"), c._write)    
}

// -----

func TestSrcpSessions_New(t *testing.T) {
    assert := assert.New(t)
    u := NewSrcpSessions()
    
    assert.NotNil(u)
    assert.Equal("", fmt.Sprintf("%v", u))
}

func TestSrcpSessions_Add(t *testing.T) {
    assert := assert.New(t)
    u := NewSrcpSessions()

    c := newMockConn( []byte{} )
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}

    u.Add(s)    
    assert.Equal("42", fmt.Sprintf("%v", u))
}

func TestSrcpSessions_Remove(t *testing.T) {
    assert := assert.New(t)
    u := NewSrcpSessions()

    c := newMockConn( []byte{} )
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}

    u.Add(s)    
    assert.Equal("42", fmt.Sprintf("%v", u))

    u.Remove(s)
    assert.Equal("", fmt.Sprintf("%v", u))
}

func TestSrcpSessions_Iter(t *testing.T) {
    assert := assert.New(t)
    u := NewSrcpSessions()

    c := newMockConn( []byte{} )
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}

    u.Add(s)
    assert.Equal(42, func()int { 
        i := 0
        u.Iter( func(s1 *SrcpSession) {
            i += s1.Id
        })
        return i
    }())
}


// -----

func _test_srcp_init(mode string, data []byte) (*Model, *MockConn, *SrcpSessions, *SrcpSession) {
    m := NewModel()
    c := newMockConn(data)
    u := NewSrcpSessions()
    s := &SrcpSession{42, mode, 11, c, u}
    u.Add(s)
    return m, c, u, s
}

func TestHandleSrcpConn_NoOp(t *testing.T) {
    m, c, _, s := _test_srcp_init(SRCP_MODE_HANDSHAKE, []byte{} )
    HandleSrcpConn(m, s)
    assert.Equal(t, []byte(SRCP_HEADER + "\n"), c._write)    
}

func TestHandleSrcpLine_SetProto(t *testing.T) {
    assert := assert.New(t)
    m, c, _, s := _test_srcp_init(SRCP_MODE_HANDSHAKE, nil)
    HandleSrcpLine(m, s, "SET PROTOCOL SRCP blah")
    assert.Equal([]byte("12 201 OK PROTOCOL SRCP\n"), c._write)    
}

func TestHandleSrcpLine_SetConnMode(t *testing.T) {
    assert := assert.New(t)
    m, c, _, s := _test_srcp_init(SRCP_MODE_HANDSHAKE, nil)
    HandleSrcpLine(m, s, "SET CONNECTIONMODE SRCP something")
    assert.Equal([]byte("12 202 OK CONNECTIONMODE\n"), c._write)    
    assert.Equal("something", s.Mode)
}

func TestHandleSrcpLine_Go_Command(t *testing.T) {
    assert := assert.New(t)
    m, c, _, s := _test_srcp_init(SRCP_MODE_HANDSHAKE, nil)
    HandleSrcpLine(m, s, "SET CONNECTIONMODE SRCP COMMAND")
    HandleSrcpLine(m, s, "GO")
    assert.Equal(
        []byte("12 202 OK CONNECTIONMODE\n" +
               "13 200 OK GO 42\n"), 
        c._write)    
}

func TestHandleSrcpLine_Go_Info(t *testing.T) {
    assert := assert.New(t)
    m, c, _, s := _test_srcp_init(SRCP_MODE_HANDSHAKE, nil)
    HandleSrcpLine(m, s, "SET CONNECTIONMODE SRCP INFO")
    HandleSrcpLine(m, s, "GO")
    assert.Equal(
        []byte("12 202 OK CONNECTIONMODE\n" +
               "13 200 OK GO 42\n" +
               "14 100 INFO 0 DESCRIPTION SESSION SERVER TIME\n" +
               "15 100 INFO 0 TIME 1 1\n" +
               "16 100 INFO 0 TIME 12345 23 59 59\n" +
               "17 100 INFO 1 POWER ON controlled by digix\n" +
               "18 100 INFO 7 DESCRIPTION GA DESCRIPTION\n" +
               "19 100 INFO 7 GA 1 0 0\n" +
               "20 100 INFO 7 GA 1 1 1\n" +
               "21 100 INFO 7 GA 2 0 0\n" +
               "22 100 INFO 7 GA 2 1 1\n" +
               "23 100 INFO 7 GA 3 0 0\n" +
               "24 100 INFO 7 GA 3 1 1\n" +
               "25 100 INFO 7 GA 4 0 0\n" +
               "26 100 INFO 7 GA 4 1 1\n" +
               "27 100 INFO 7 GA 5 0 0\n" +
               "28 100 INFO 7 GA 5 1 1\n" +
               "29 100 INFO 7 GA 6 0 0\n" +
               "30 100 INFO 7 GA 6 1 1\n" +
               "31 100 INFO 7 GA 7 0 0\n" +
               "32 100 INFO 7 GA 7 1 1\n" +
               "33 100 INFO 7 GA 8 0 0\n" +
               "34 100 INFO 7 GA 8 1 1\n" +
               "35 100 INFO 7 GA 9 0 0\n" +
               "36 100 INFO 7 GA 9 1 1\n" +
               "37 100 INFO 7 GA 10 0 0\n" +
               "38 100 INFO 7 GA 10 1 1\n" +
               "39 100 INFO 7 GA 11 0 0\n" +
               "40 100 INFO 7 GA 11 1 1\n" +
               "41 100 INFO 7 GA 12 0 0\n" +
               "42 100 INFO 7 GA 12 1 1\n" +
               "43 100 INFO 7 GA 13 0 0\n" +
               "44 100 INFO 7 GA 13 1 1\n" +
               "45 100 INFO 7 GA 14 0 0\n" +
               "46 100 INFO 7 GA 14 1 1\n" +
               "47 100 INFO 7 GA 15 0 0\n" +
               "48 100 INFO 7 GA 15 1 1\n" +
               "49 100 INFO 7 GA 16 0 0\n" +
               "50 100 INFO 7 GA 16 1 1\n" +
               "51 100 INFO 7 GA 17 0 0\n" +
               "52 100 INFO 7 GA 17 1 1\n" +
               "53 100 INFO 7 GA 18 0 0\n" +
               "54 100 INFO 7 GA 18 1 1\n" +
               "55 100 INFO 7 GA 19 0 0\n" +
               "56 100 INFO 7 GA 19 1 1\n" +
               "57 100 INFO 7 GA 20 0 0\n" +
               "58 100 INFO 7 GA 20 1 1\n" +
               "59 100 INFO 7 GA 21 0 0\n" +
               "60 100 INFO 7 GA 21 1 1\n" +
               "61 100 INFO 7 GA 22 0 0\n" +
               "62 100 INFO 7 GA 22 1 1\n" +
               "63 100 INFO 7 GA 23 0 0\n" +
               "64 100 INFO 7 GA 23 1 1\n" +
               "65 100 INFO 7 GA 24 0 0\n" +
               "66 100 INFO 7 GA 24 1 1\n" +
               "67 100 INFO 7 GA 25 0 0\n" +
               "68 100 INFO 7 GA 25 1 1\n" +
               "69 100 INFO 7 GA 26 0 0\n" +
               "70 100 INFO 7 GA 26 1 1\n" +
               "71 100 INFO 7 GA 27 0 0\n" +
               "72 100 INFO 7 GA 27 1 1\n" +
               "73 100 INFO 7 GA 28 0 0\n" +
               "74 100 INFO 7 GA 28 1 1\n" +
               "75 100 INFO 8 DESCRIPTION FB DESCRIPTION\n" +
               "76 100 INFO 8 FB 1 0\n" +
               "77 100 INFO 8 FB 2 0\n" +
               "78 100 INFO 8 FB 3 0\n" +
               "79 100 INFO 8 FB 4 0\n" +
               "80 100 INFO 8 FB 5 0\n" +
               "81 100 INFO 8 FB 6 0\n" +
               "82 100 INFO 8 FB 7 0\n" +
               "83 100 INFO 8 FB 8 0\n" +
               "84 100 INFO 8 FB 9 0\n" +
               "85 100 INFO 8 FB 10 0\n" +
               "86 100 INFO 8 FB 11 0\n" +
               "87 100 INFO 8 FB 12 0\n" +
               "88 100 INFO 8 FB 13 0\n" +
               "89 100 INFO 8 FB 14 0\n" +
               "90 100 INFO 8 FB 15 0\n" +
               "91 100 INFO 8 FB 16 0\n" +
               "92 100 INFO 8 FB 17 0\n" +
               "93 100 INFO 8 FB 18 0\n" +
               "94 100 INFO 8 FB 19 0\n" +
               "95 100 INFO 8 FB 20 0\n" +
               "96 100 INFO 8 FB 21 0\n" +
               "97 100 INFO 8 FB 22 0\n" +
               "98 100 INFO 8 FB 23 0\n" +
               "99 100 INFO 8 FB 24 0\n" +
               "100 100 INFO 8 FB 25 0\n" +
               "101 100 INFO 8 FB 26 0\n" +
               "102 100 INFO 8 FB 27 0\n" +
               "103 100 INFO 8 FB 28 0\n" +
               "104 100 INFO 8 FB 29 0\n" +
               "105 100 INFO 8 FB 30 0\n" +
               "106 100 INFO 8 FB 31 0\n" +
               "107 100 INFO 8 FB 32 0\n" +
               "108 100 INFO 8 FB 33 0\n" +
               "109 100 INFO 8 FB 34 0\n" +
               "110 100 INFO 8 FB 35 0\n" +
               "111 100 INFO 8 FB 36 0\n" +
               "112 100 INFO 8 FB 37 0\n" +
               "113 100 INFO 8 FB 38 0\n" +
               "114 100 INFO 8 FB 39 0\n" +
               "115 100 INFO 8 FB 40 0\n" +
               "116 100 INFO 8 FB 41 0\n" +
               "117 100 INFO 8 FB 42 0\n" +
               "118 100 INFO 8 FB 43 0\n" +
               "119 100 INFO 8 FB 44 0\n" +
               "120 100 INFO 8 FB 45 0\n" +
               "121 100 INFO 8 FB 46 0\n" +
               "122 100 INFO 8 FB 47 0\n" +
               "123 100 INFO 8 FB 48 0\n" +
               "124 100 INFO 8 FB 49 0\n" +
               "125 100 INFO 8 FB 50 0\n" +
               "126 100 INFO 8 FB 51 0\n" +
               "127 100 INFO 8 FB 52 0\n" +
               "128 100 INFO 8 FB 53 0\n" +
               "129 100 INFO 8 FB 54 0\n" +
               "130 100 INFO 8 FB 55 0\n" +
               "131 100 INFO 8 FB 56 0\n"), 
        c._write)    
}

