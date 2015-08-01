package translate

import (
    "testing"
    "github.com/stretchr/testify/assert"
)

func TestSrcpSession_Reply(t *testing.T) {
    assert := assert.New(t)
    c := newMockConn(nil)
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c}
    
    assert.Equal(42, s.Id)
    assert.Equal(11, s.Time)
    assert.Equal(SRCP_MODE_HANDSHAKE, s.Mode)
    
    s.Reply("SRCP PROTOCOL")
    assert.Equal(12, s.Time)
    assert.Equal( []byte("12 SRCP PROTOCOL\n"), c._write)    
}

func TestHandleSrcpConn_Foo(t *testing.T) {
    m := NewModel()
    c := newMockConn( []byte{ OP_GET_VERSION } )
    s := &SrcpSession{42, SRCP_MODE_HANDSHAKE, 11, c}
    sessions := map[int] *SrcpSession { s.Id: s }

    HandleSrcpConn(m, s, sessions)
}

