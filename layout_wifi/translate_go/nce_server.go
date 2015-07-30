package main

import (
    "fmt"
    //"io"
    "net"
)

const NCE_PORT = ":8080"

func NceServer() {
    fmt.Println("Start NCE server")

    listener, err := net.Listen("tcp", NCE_PORT) // TODO Constant
    if err != nil {
        panic(err)
    }
    
    go func(listener net.Listener) {
        defer listener.Close()
        
        for !IsQuitting() {
            conn, err := listener.Accept()
            if err != nil {
                panic(err)
            }
            go HandleNceConn(conn)
        }
    }(listener)
}

func HandleNceConn(conn net.Conn) {
    defer conn.Close()
    fmt.Println("[NCE] New connection")
        
    buf := make([]byte, 16)

    loopRead: for {
        n, err := conn.Read(buf[0:1])

        if n == 1 && err == nil {
            cmd := buf[0]
            fmt.Printf("[NCE] Command: 0x%02x\n", cmd)
            
            switch cmd {
            case 0xAA:
                // reply with version 6.3.8
                fmt.Println("[NCE] > Version")
                buf[0] = 6
                buf[1] = 3
                buf[2] = 8
                n, err = conn.Write(buf[0:3])
            default:
                fmt.Printf("[NCE] > Ignored command 0x%02x\n", cmd)
            }
        }
        if err != nil {
            if op, ok := err.(*net.OpError); ok {
                fmt.Println("[NCE] Connection error:", op.Op)
                break loopRead
            } else {
                panic(err)
            }
        }
    }
    fmt.Println("[NCE] Connection closed")
}
