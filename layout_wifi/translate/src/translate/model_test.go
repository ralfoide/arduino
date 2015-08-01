package translate

import (
    "testing"
    "github.com/stretchr/testify/assert"
)

func TestQuitting(t *testing.T) {
    m := NewModel()

    assert.Equal(t, false, m.IsQuitting())

    m.SetQuitting()
    assert.Equal(t, true, m.IsQuitting())
}

func TestSensors(t *testing.T) {
    assert := assert.New(t)
    m := NewModel()

    for i := 1; i <= MAX_AIUS; i++ {
        assert.Equal(uint16(0), m.GetSensors(i))

        m.SetSensors(i, uint16(i))
        assert.Equal(uint16(i), m.GetSensors(i))
    }
}

func TestTurnoutOp(t *testing.T) {
    assert := assert.New(t)
    m := NewModel()

    op, ok := m.GetTurnoutOp()
    assert.Equal(false, ok)
    assert.Nil(op)

    m.SendTurnoutOp( &TurnoutOp{6, true} )
    
    op, ok = m.GetTurnoutOp()
    assert.Equal(true, ok)
    assert.Equal(TurnoutOp{6, true}, *op)    
}

func TestTurnoutStates(t *testing.T) {
    assert := assert.New(t)
    m := NewModel()

    assert.Equal(0x0000, m.GetTurnoutStates())
    
    m.SetTurnoutState(1, true)
    m.SetTurnoutState(2, false)
    m.SetTurnoutState(4, true)
    m.SetTurnoutState(5, false)
    
    assert.Equal(0x0022, m.GetTurnoutStates())
}
