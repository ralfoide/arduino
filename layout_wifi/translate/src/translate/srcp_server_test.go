package translate

import (
    "fmt"
    "testing"
    "github.com/stretchr/testify/assert"
)

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

func TestHandleSrcpConn_NoOp(t *testing.T) {
    m := NewModel()
    c := newMockConn( []byte{} )
    u := NewSrcpSessions()
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c, u}
    u.Add(s)
    HandleSrcpConn(m, s)
    u.Remove(s)
}

