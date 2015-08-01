package translate

import (
    "bufio"
    "fmt"
    "io"
    "os"
    "os/signal"
    "syscall"
)

func SetupSignal(m *Model) {
    c := make(chan os.Signal, 1)
    signal.Notify(c, os.Interrupt)  // Ctrl-C
    signal.Notify(c, syscall.SIGTERM) // kill -9
    go func(m *Model) {
        <-c
        fmt.Println("Caught Ctrl-C")
        m.SetQuitting()
    }(m)
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

func TerminalLoop(m *Model) {
    fmt.Println("Enter terminal")

    for !m.IsQuitting() {
        str := ReadLine(os.Stdin)

        if str == "quit" || str == "q" {
            m.SetQuitting()
        } else {
            fmt.Println("Unknown command. Use quit or q.")
        }
    }
    fmt.Println("Exiting")
}

func Main() {
    model := NewModel()
    SetupSignal(model)
    NceServer(model)
    TerminalLoop(model)
}

