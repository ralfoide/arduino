package translate

import (
    "bufio"
    "fmt"
    "net"
    "strconv"
    "strings"
    "sync"
    "time"
)

const SRCP_PORT           = ":4303"
const SRCP_MODE_HANDSHAKE = "HANDSHAKE"
const SRCP_MODE_INFO      = "INFO"
const SRCP_MODE_COMMAND   = "COMMAND"
const SRCP_HEADER         = "SRCP 0.8.4; SRCPOTHER 0.8.3"

const SRCP_BUS_TURNOUTS   = 7 // bus used by SRCP Client to control the turnouts
const SRCP_BUS_SENSORS    = 8 // bus used by SRCP Client to poll sensors

//-----

type SrcpSession struct {
    Id       int
    Mode     string
    Time     int
    Conn     net.Conn
    Sessions *SrcpSessions
}

func (s *SrcpSession) ReplyRaw(str string) {
    str = fmt.Sprintf("%s\n", str)
    fmt.Printf("[SRCP %d] < %s", s.Id, str)
    s.Conn.Write( []byte(str) )
}

func (s *SrcpSession) Reply(str string) {
    s.Time += 1
    s.ReplyRaw(fmt.Sprintf("%d %s", s.Time, str))
}

//-----

type SrcpSessions struct {
    mutex    sync.Mutex
    sessions map[int] *SrcpSession
}

func NewSrcpSessions() *SrcpSessions {
    s := &SrcpSessions{}
    s.sessions = make(map[int] *SrcpSession)
    return s
}

func (s *SrcpSessions) Add(session *SrcpSession) {
    s.mutex.Lock()
    defer s.mutex.Unlock()

    s.sessions[session.Id] = session
}

func (s *SrcpSessions) Remove(session *SrcpSession) {
    s.mutex.Lock()
    defer s.mutex.Unlock()

    delete(s.sessions, session.Id)
}

func (s *SrcpSessions) Iter(f func(*SrcpSession)) {
    s.mutex.Lock()
    defer s.mutex.Unlock()

    for _, v := range s.sessions {
        if v != nil {
            f(v)
        }
    }
}

func (s *SrcpSessions) String() string {
    str := ""

    s.mutex.Lock()
    defer s.mutex.Unlock()

    for k, v := range s.sessions {
        if v != nil {
            str += strconv.Itoa(k) + " "
        }
    }
    
    return strings.TrimSpace(str)
}

//-----

func SrcpServer(m *Model) {
    fmt.Println("Start SRCP server")

    listener, err := net.Listen("tcp", SRCP_PORT)
    if err != nil {
        panic(err)
    }
    
    go func(listener net.Listener) {
        defer listener.Close()

        sessions := NewSrcpSessions()
        session_id := 0
        
        for !m.IsQuitting() {
            conn, err := listener.Accept()
            if err != nil {
                panic(err)
            }
            session_id += 1
            go func(id int, conn net.Conn) {
                session := &SrcpSession{id, SRCP_MODE_HANDSHAKE, 0, conn, sessions}
                sessions.Add(session)
                defer sessions.Remove(session)
                HandleSrcpConn(m, session)
            }(session_id, conn)
        }
    }(listener)
}

func HandleSrcpConn(m *Model, s *SrcpSession) {
    fmt.Println("[SRCP] New session/connection")

    conn := s.Conn
    defer conn.Close()

    s.ReplyRaw(SRCP_HEADER)
    
    r := bufio.NewReader(conn)

    loopRead: for !m.IsQuitting() {
        if s.Mode == SRCP_MODE_INFO {
            s.Conn.SetReadDeadline(time.Now().Add(500 * time.Millisecond))
        }
        line, err := r.ReadString('\n')

        if s.Mode == SRCP_MODE_INFO && err != nil {
            if e, ok := err.(*net.OpError); ok && e.Timeout() {
                // TODO send periodic INFO sensor updates
                // continue loopRead
            }
        }

        if err == nil {
            line := strings.TrimSpace(line)
            fmt.Printf("[SRCP %d] > %s\n", s.Id, line)
            err = HandleSrcpLine(m, s, line)
        }

        if err != nil {
            if op, ok := err.(*net.OpError); ok {
                fmt.Println("[SRCP] Connection error:", op.Op)
                break loopRead
            } else {
                panic(err)
            }
        }
    }
    
    fmt.Println("[SRCP] Connection closed")
}

func HandleSrcpLine(m *Model, s *SrcpSession, line string) error {

    if strings.Index(line, "SET PROTOCOL SRCP ") == 0 {
        s.Reply("201 OK PROTOCOL SRCP")
        
    } else if strings.Index(line, "SET CONNECTIONMODE SRCP ") == 0 {
        s.Mode = line[ len("SET CONNECTIONMODE SRCP ") : ]
        s.Reply("202 OK CONNECTIONMODE")
                
    } else if line == "GO" {
        s.Reply("200 OK GO " + strconv.Itoa(s.Id))
        if s.Mode == SRCP_MODE_INFO {
            s.Reply("100 INFO 0 DESCRIPTION SESSION SERVER TIME")

            // time ratio fx:fy
            s.Reply("100 INFO 0 TIME 1 1")
            // time julianDay HH MM SS... just mock it
            s.Reply("100 INFO 0 TIME 12345 23 59 59")

            // power is always on and not really controllable
            s.Reply("100 INFO 1 POWER ON controlled by digix")

            // bus 7 is SRCP_BUS_TURNOUTS
            s.Reply("100 INFO 7 DESCRIPTION GA DESCRIPTION")
            // give the initial turnout states
            states := m.GetTurnoutStates()
            for t := 1; t <= MAX_TURNOUTS; t++ {
                normal := (states & 1)
                states >>= 1
                // turnout N port 0 for normal, value 1 if normal
                // turnout N port 1 for reverse, value 1 if reverse
                s.Reply(fmt.Sprintf("100 INFO 7 GA %d 0 %d", t, normal))
                s.Reply(fmt.Sprintf("100 INFO 7 GA %d 1 %d", t, 1 - normal))
            }

            // bus 8 is SRCP_BUS_SENSORS
            s.Reply("100 INFO 8 DESCRIPTION FB DESCRIPTION")
            // give the initial sensors states
            for t := 1; t <= MAX_SENSORS; t++ {               
                b := 0
                if m.GetSensor(t) {
                    b = 1
                }
                s.Reply(fmt.Sprintf("100 INFO 8 FB %d %d", t, b))
            }
        }
    }

    return nil
}
