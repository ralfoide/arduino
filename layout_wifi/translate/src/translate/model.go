package translate

import (
    "fmt"
    "sync"
    "sync/atomic"
)

const CONTINUE = 0
const QUITTING = 1

const MAX_AIUS  = 8
const SENSORS_PER_AIU = 14
const MAX_SENSORS = SENSORS_PER_AIU * MAX_AIUS

// Expect to handle 8 turnouts, using 32 for future expansion.
const MAX_TURNOUTS = 32

type TurnoutOp struct {
    // Turnout Addresses are 1-based: 1..MAX_TURNOUTS
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
    
    m.sensors = make([]uint16, MAX_AIUS)
    return m
}

func (m *Model) SetQuitting() {
    atomic.StoreInt32(&m.quitting, QUITTING)
}

func (m *Model) IsQuitting() bool {
    return atomic.LoadInt32(&m.quitting) == QUITTING
}

// Get the 14-bit sensor value for the given AIU.
// AIU numbers are 1-based: 1..MAX_AIUS
func (m *Model) GetSensors(aiu int) uint16 {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    if (aiu < 1 || aiu > MAX_AIUS) {
        panic(fmt.Errorf("Invalid AIU number %d [1..%d]", aiu, MAX_AIUS))
    }

    return m.sensors[aiu - 1]
}

// Set the 14-bit sensors value for the given AIU.
// AIU numbers are 1-based: 1..MAX_AIUS
func (m *Model) SetSensors(aiu int, sensors uint16) {
    m.mutex.Lock()
    defer m.mutex.Unlock()

    if (aiu < 1 || aiu > MAX_AIUS) {
        panic(fmt.Errorf("Invalid AIU number %d [1..%d]", aiu, MAX_AIUS))
    }

    m.sensors[aiu - 1] = sensors
}

func (m *Model) SendTurnoutOp(op *TurnoutOp) {
    if (op.Address < 1 || op.Address > MAX_TURNOUTS) {
        panic(fmt.Errorf("Invalid turnout number %d [1..%d]", op.Address, MAX_TURNOUTS))
    }

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
    if (index < 1 || index > MAX_TURNOUTS) {
        panic(fmt.Errorf("Invalid turnout index %d [1..%d]", index, MAX_TURNOUTS))
    }
    index -= 1

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

