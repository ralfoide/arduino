package translate

import (
    "testing"
    "github.com/stretchr/testify/assert"
)

// -----

func TestLwServ_New(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()
    
    assert.NotNil(s)
}

func TestHandleLwServLine_NoOp(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()

    assert.Equal("", HandleLwServLine(s, ""))
}

func TestHandleLwServLine_ReceiveInfo(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()

    assert.Equal("@IT04S01\n", HandleLwServLine(s, "@I"))
}

func TestHandleLwServLine_TurnoutNormal(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()

    assert.Equal(uint8(0), s._turnouts[0])
    assert.Equal("@T01N\n", HandleLwServLine(s, "@T01N"))
    assert.Equal(uint8('N'), s._turnouts[0])
}

func TestHandleLwServLine_TurnoutReverse(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()

    assert.Equal(uint8(0), s._turnouts[0])
    assert.Equal("@T01R\n", HandleLwServLine(s, "@T01R"))
    assert.Equal(uint8('R'), s._turnouts[0])
}

func TestLwServPollSensors(t *testing.T) {
    assert := assert.New(t)
    s := NewLwServ()
    
    assert.Equal("@S010000\n", LwServPollSensors(s))

    s.sensors_chan <- LwSensor{ 1,  true}
    s.sensors_chan <- LwSensor{ 2,  true}
    s.sensors_chan <- LwSensor{ 2, false}
    s.sensors_chan <- LwSensor{14,  true}

    assert.Equal("@S012001\n", LwServPollSensors(s))
}
