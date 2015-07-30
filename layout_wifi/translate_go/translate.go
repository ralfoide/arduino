package main

import (
    "bufio"
    "fmt"
    "io"
    "os"
    "os/signal"
    //"time"
    "syscall"
    "sync/atomic"
)

const CONTINUE = 0
const QUITTING = 1

var Quitting int32 = CONTINUE

func SetQuitting() {
    atomic.StoreInt32(&Quitting, QUITTING)
}

func IsQuitting() bool {
    return atomic.LoadInt32(&Quitting) == QUITTING
}

func SetupSignal() {
    c := make(chan os.Signal, 1)
    signal.Notify(c, os.Interrupt)  // Ctrl-C
    signal.Notify(c, syscall.SIGTERM) // kill -9
    go func() {
        <-c
        fmt.Println("Caught Ctrl-C")
        SetQuitting()
    }()
}

func ReadLine(in io.ReadWriter) string {
    fmt.Print("> ")
    b := bufio.NewReader(in)
    line, _ /*hasMore*/, err := b.ReadLine()
    if err != nil {
        panic(err)
    }
    return string(line)
}

func TerminalLoop() {
    fmt.Println("Enter terminal")

    for !IsQuitting() {
        str := ReadLine(os.Stdin)

        if str == "quit" || str == "q" {
            SetQuitting()
        } else {
            fmt.Println("Unknown command. Use quit or q.")
        }
    }
    fmt.Println("Exiting")
}

func main() {
    SetupSignal()
    NceServer()
    TerminalLoop()
}

