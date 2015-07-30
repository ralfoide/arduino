package translate

import (
    "io"
    "net"
    "testing"
    "time"
    "utils"
    "github.com/stretchr/testify/assert"
)

// ----

type MockConn struct {
    _read   []byte
    _write  []byte 
    _closed bool 
}

func newMockConn(_read []byte) *MockConn {
    c := &MockConn{}
    c._read = _read
    return c
}

func (c *MockConn) Close() error {
    return nil
}

func (c *MockConn) Read(b []byte) (n int, err error) {
    n = utils.MinInt(len(b), len(c._read))
    if n > 0 {
        copy(b, c._read[0:n])
        c._read = c._read[n:]
        return n, nil

    } else {    
        return n, &net.OpError{"EOF", "tcp", nil, io.EOF}
    }
}

func (c *MockConn) Write(b []byte) (n int, err error) {
    return 0, nil
}

func (c *MockConn) LocalAddr() net.Addr {
    return nil
}

func (c *MockConn) RemoteAddr() net.Addr {
    return nil
}

func (c *MockConn) SetDeadline(t time.Time) error {
    return nil
}

func (c *MockConn) SetReadDeadline(t time.Time) error {
    return nil
}

func (c *MockConn) SetWriteDeadline(t time.Time) error {
    return nil
}

// ---

func TestMockConnRead(t *testing.T) {
    assert := assert.New(t)
    c := newMockConn( []byte{ 1, 2, 3, 4, 5 } )

    buf2 := make([]byte, 2)
    buf3 := make([]byte, 3)
    
    n, err := c.Read(buf2)
    assert.Equal(2, n)
    assert.Nil(err)
    
    n, err = c.Read(buf3)
    assert.Equal(3, n)
    assert.Nil(err)
    
    n, err = c.Read(buf2)
    assert.Equal(0, n)
    assert.NotNil(t, err)
}

func TestHandleNceConn(t *testing.T) {
    c := newMockConn( []byte{ 0xAA } )
    HandleNceConn(c)
}
