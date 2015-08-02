package translate 

import (
    "bufio"
    "flag"
    "fmt"
    "net"
    "strings"
    "time"
)

var LW_SERV_PORT = flag.String("lw-serv-port", ":9090", "LayoutWifi simulator server host:port")

const LW_LED_PIN        = 13
const LW_RELAY_PIN_1    = 90
const LW_TURNOUT_N      =  4
const LW_RELAY_N        =  2 * LW_TURNOUT_N

const LW_SENSOR_PIN_1   = 107
const LW_SENSOR_N       =  14
const LW_AIU_N          =   1

const LW_TURNOUT_NORMAL    = 'N'
const LW_TURNOUT_REVERSE   = 'R'

func LW_RELAY_NORMAL(T int) int {
    return 2 * (T)
}

func LW_RELAY_REVERSE(T int) int {
    return ((2 * (T)) + 1)
}

func LW_READ_BIT( I  uint16, N uint) uint16 {
    return (((I) &  (1<<N)) >> N)
}
func LW_SET_BIT(  I  uint16, N uint) uint16 {
    return ((I) |  (1<<N))
}
func LW_CLEAR_BIT(I  uint16, N uint) uint16 {
    return ((I) & ^(1<<N))
}

//-----

type LwSensor struct {
    Sensor  uint
    Active  bool
}

type LwServ struct {
    _sensors    []uint16
    _turnouts   []uint8
    sensors_chan chan LwSensor
}

func NewLwServ() *LwServ {
    lw := &LwServ{}
    lw._sensors = make([]uint16, LW_AIU_N)
    lw._turnouts = make([]uint8, LW_TURNOUT_N)
    lw.sensors_chan = make(chan LwSensor, 32)
    return lw
}

//-----

func LwServer(m *Model) chan LwSensor {
    fmt.Println("Start LW server")

    listener, err := net.Listen("tcp", *LW_SERV_PORT)
    if err != nil {
        panic(err)
    }
    
    serv := NewLwServ()
    
    go func(listener net.Listener) {
        defer listener.Close()

        for !m.IsQuitting() {
            conn, err := listener.Accept()
            if err != nil {
                panic(err)
            }
            go func(conn net.Conn) {
                HandleLwServConn(conn, m, serv)
            }(conn)
        }
    }(listener)
    
    return serv.sensors_chan
}

func HandleLwServConn(conn net.Conn, m *Model, s *LwServ) {
    fmt.Println("[LW-SERV] New connection")

    defer conn.Close()

    r := bufio.NewReader(conn)

    loopRead: for !m.IsQuitting() {
        conn.SetReadDeadline(time.Now().Add(500 * time.Millisecond))
        line, err := r.ReadString('\n')

        if err != nil {
            if e, ok := err.(*net.OpError); ok && e.Timeout() {
                reply := LwServPollSensors(s)
                if len(reply) > 0 {
                    _, err = conn.Write( []byte(reply) )
                }
                continue loopRead
            }
        }

        if err == nil {
            line := strings.TrimSpace(line)
            fmt.Printf("[LW-SERV] > %s\n", line)
            reply := HandleLwServLine(s, line)
            if len(reply) > 0 {
                _, err = conn.Write( []byte(reply) )
            }
        }

        if err != nil {
            if op, ok := err.(*net.OpError); ok {
                fmt.Println("[LW-SERV] Connection error:", op.Op)
                break loopRead
            } else {
                panic(err)
            }
        }
    }
    
    fmt.Println("[LW-SERV] Connection closed")
}

func HandleLwServLine(s *LwServ, line string) string {

    if line == "@I" {
        // Info command: @I\n
        fmt.Println("[LW-SERV] Info Cmd");
        // Reply: @IT<00>S<00>\n
        return fmt.Sprintf("@IT%02dS%02d\n", LW_TURNOUT_N, LW_AIU_N);
    }
    
    if len(line) == 5 && strings.HasPrefix(line, "@T") {
        // Turnout command: @T<00><N|R>\n
        buf := line[2:5]
        turnout := (int(buf[0] - '0') << 8) + int(buf[1] - '0')
        direction := uint8(buf[2])
        if ((direction == LW_TURNOUT_NORMAL || direction == LW_TURNOUT_REVERSE) &&
                turnout > 0 && turnout <= LW_TURNOUT_N) {
            fmt.Printf("[LW-SERV] Accepted Turnout Cmd %d = %c\n", turnout, direction);
            turnout -= 1 // cmd index is 1-based but array & relays are 0-based
            if s._turnouts[turnout] != direction {
                if direction == LW_TURNOUT_NORMAL {
                    fmt.Printf("[LW-SERV] Simulate trigger pin %d\n", LW_RELAY_NORMAL(turnout))
                } else {
                    fmt.Printf("[LW-SERV] Simulate trigger pin %d\n", LW_RELAY_REVERSE(turnout))
                }
                s._turnouts[turnout] = direction
                // Reply is the same command
                return line + "\n"
            }
        } else {
            fmt.Printf("[LW-SERV] Rejected Turnout Cmd '%s'\n", line);
        }
    }
    
    fmt.Printf("[LW-SERV] Unknown Cmd: '%s'\n", line)

    return ""
}

func LwServPollSensors(lw *LwServ) (reply string) {
    loopRead: for {
        select {
        case s := <- lw.sensors_chan:
            index := s.Sensor
            if index < 1 || index > LW_SENSOR_N {
                panic(fmt.Errorf("Invalid LW sensor index %d [1..%d]", index, LW_SENSOR_N))
            }
            index -= 1
            aiu   := index / SENSORS_PER_AIU
            index  = index % SENSORS_PER_AIU

            if s.Active {
                lw._sensors[aiu] = LW_SET_BIT(lw._sensors[aiu], index)
                fmt.Printf("[LW-SERV] Simulate set sensor %d:%d ON\n", aiu+1, index+1)
            } else {
                lw._sensors[aiu] = LW_CLEAR_BIT(lw._sensors[aiu], index)
                fmt.Printf("[LW-SERV] Simulate set sensor %d:%d OFF\n", aiu+1, index+1)
            }
        default:
            break loopRead
        }
    }

    for aiu := 0; aiu < LW_AIU_N; aiu++ {    
        s := lw._sensors[aiu]
        reply += fmt.Sprintf("@S%02d%04x\n", aiu + 1, s)
    }    
    
    return reply
}
