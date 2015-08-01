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

    for i := 0; i < MAX_AIU; i++ {
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

    m.SendTurnoutOp( &TurnoutOp{42, true} )
    
    op, ok = m.GetTurnoutOp()
    assert.Equal(true, ok)
    assert.Equal(TurnoutOp{42, true}, *op)    
}

func TestTurnoutStates(t *testing.T) {
    assert := assert.New(t)
    m := NewModel()

    assert.Equal(0x0000, m.GetTurnoutStates())
    
    m.SetTurnoutState(0, true)
    m.SetTurnoutState(1, false)
    m.SetTurnoutState(8, true)
    m.SetTurnoutState(9, false)
    
    assert.Equal(0x0202, m.GetTurnoutStates())
}
