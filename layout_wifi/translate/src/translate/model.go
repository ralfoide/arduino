package translate

import (
    "sync"
    "sync/atomic"
)

const CONTINUE = 0
const QUITTING = 1

const MAX_AIU  = 8
const MAX_TURNOUT = 8

type TurnoutOp struct {
    Address int
    Normal  bool
}

type Model struct {
    quitting int32  // Atomic access only

    turnoutOps chan *TurnoutOp
    
    // All fields below access only when the Mutex is held
    mutex   sync.Mutex

    // One uint16 per AIU, which carry 14-bits each.
    sensors []uint16
    
    // Turnout state, one bit per turnout.
    // Bit set to 0 means normal, set to 1 means reverse.
    // Expect to handle 8 turnouts, using 32-bits for future expansion.
    turnouts uint32
}

func NewModel() *Model {
    m := &Model{}
    m.quitting = CONTINUE
    m.turnoutOps = make(chan *TurnoutOp, 16)
    
    m.mutex.Lock()
    defer m.mutex.Unlock()
    
    m.sensors = make([]uint16, MAX_AIU)
    return m
}

func (m *Model) SetQuitting() {
    atomic.StoreInt32(&m.quitting, QUITTING)
}

func (m *Model) IsQuitting() bool {
    return atomic.LoadInt32(&m.quitting) == QUITTING
}

func (m *Model) GetSensors(aiu int) uint16 {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    return m.sensors[aiu]
}

func (m *Model) SetSensors(aiu int, sensors uint16) {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    m.sensors[aiu] = sensors
}

func (m *Model) SendTurnoutOp(op *TurnoutOp) {
    m.turnoutOps <- op
}

func (m *Model) GetTurnoutOp() (op *TurnoutOp, ok bool) {
    select {
    case op = <- m.turnoutOps:
        ok = true
    default:
        op = nil
        ok = false
    }
    return op, ok
}

func (m *Model) SetTurnoutState(index uint, normal bool) {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    if normal {
        m.turnouts &= ^(1 << index)
    } else {
        m.turnouts |= 1 << index
    }
}

func (m *Model) GetTurnoutStates() uint32 {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    return m.turnouts
}

/* do we really need this?
func (m *Model) GetTurnoutState(index int) (normal bool) {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    return (m.turnouts & (1 << index)) == 0
}
*/

