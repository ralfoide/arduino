package main

/*
  Translator program.

  JRMI --[NCE binary protocol]-->|
                                 |<--> Translator <--> DigiX LayoutWifi
  RocRail --[SRCP protocol]----->|
  
  TODO: find how to send turnout feedback to JRMI or RocRail.

  Architecture (without turnout feedback so far):
  - Shared model: sensor state, turnout states (atomic access.)
  - Slice of channels to receive sensor-updates.  [maybe?]
  - Slice of channels to receive turnout-updates. [maybe?]

  - DigiX client (go loop).
    - Argument: channel to receive turnout commands.
    - Keep a permanent connection to the digix, re-opening it once closed.
        - Do *once* an Info command to get the number of turnouts/sensors.
            then release the init channel.
        - Polls turnout channel and emits turnouts commands when needed.
            - Don't update internal turnout state, this is done from reading the DigiX.
        - Read from DigiX and update internal sensor/turnout state (atomic.)

  - NCE server (go loop) on port 8080
    - Spawn go handlers for each connection.
        - Argument: channel to send turnout commands.
        - Keep last-sensor value state in each cnx.
        - Process turnout command by sending to turnout channel.
        - Process sensor command by reading atomic sensor value + compute mask.
        - Process turnout feedback by sending atomic turnout value.

  - SRCP server (go loop) on port 4303
    - Spawn go handlers for each connection.
        - Argument: channel to send turnout commands
        - Loop on SRCP requests, info mode vs command mode.
        - In command mode, process turnout commands by sending to turnout channel.
        - In info mode:
            - Process incoming command if any
            - If sensor channel has data, check if update (vs internal state) and send it.
            - If turnout feedback channel has data, check if update (vs internal state) and send it.
    
  
*/

import "fmt"


func digix_client(server_init chan bool) {
	fmt.Printf("Start DigiX Client.\n")
    server_init <- true
}

func nce_server(server_init, server_end chan bool) {
	fmt.Printf("Start NCE Server.\n")
    server_init <- true
	fmt.Printf("End NCE Server.\n")
    server_end <- true
}

func srcp_server(server_init, server_end chan bool) {
	fmt.Printf("Start SRCP Server.\n")
    server_init <- true
	fmt.Printf("End SRCP Server.\n")
    server_end <- true
}

func main() {
	fmt.Printf("Start.\n")

    server_init := make(chan bool)
    server_end := make(chan bool)

    go digix_client(server_init);
    <-server_init

    go nce_server(server_init, server_end);
    <-server_init

    go srcp_server(server_init, server_end);
    <-server_init

    // All 3 client/server initialized. Wait till one server finishes.    
    <-server_end

	fmt.Printf("End.\n")
}
