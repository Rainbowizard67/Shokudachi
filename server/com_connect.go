package server

import (
	"fmt"
	"log"
	"sync"

	"go.bug.st/serial"
)

func SetSerialConnection(mode *serial.Mode) serial.Port {
	ports, err := serial.GetPortsList()
	if err != nil {
		log.Fatal(err)
	}
	if len(ports) == 0 {
		log.Fatal("No serial ports found!")
	}
	for _, port := range ports {
		fmt.Printf("Found port: %v\n", port)
	}

	port, err := serial.Open(ports[0], mode)
	if err != nil {
		log.Fatal(err)
	}

	return port
}

func StartMainListener(port serial.Port) error {
	buffer := make([]byte, 100) //Change to match max struct settings

	var mu sync.Mutex
	var wg sync.WaitGroup

	for {
		n, err := port.Read(buffer)
		if err != nil {
			log.Fatal(err)
		}
		if n == 0 {
			fmt.Println("\nEOF")
			break
		}

		go
	}

	return nil
}
