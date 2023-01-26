#include "RalfDigiFi.h"
#include <ctype.h>

#define LED_PIN     13
#define RELAY_PIN_1 90
#define RELAY_N      8
#define IN_PIN_1   107
#define IN_N         1

DigiFi _wifi;

int _relay = 0;
unsigned long _blink_ms = 0;
char _buf_1024[1024];

// --------------------------------

void setup() {
    setup_pause();
    setup_wifi();
    setup_inputs();
    setup_relays();
}

void setup_pause() {
    //DigiX trick - since we are on serial over USB wait for character to be entered in serial terminal
    // Delay start by up to 5 seconds unless a key is pressed.
    // This should give enough to reprogram a bad behaving program.
    for (int i = 0; i < 5 && !Serial.available(); i++) {
        Serial.println("Enter any key to begin");
        blink();
        delay(1000);
    }
}

void setup_wifi() {
    Serial.begin(9600); 
    _wifi.setDebug(true);
    _wifi.begin(9600);

    Serial.println("Starting");

    while (_wifi.ready() != 1) {
        Serial.println("Error connecting to network");
        delay(15000);
    }  

    Serial.println("Connected to wifi!");
    Serial.print("Server running at: ");
    String address = _wifi.server(8080); //sets up server and returns IP
    Serial.println(address); 
}

void setup_inputs() {
    for (int i = 0; i < IN_N; i++) {
        pinMode(IN_PIN_1, INPUT_PULLUP);
    }
}

void setup_relays() {
    // initialize digital pin 90-97 as an output.
    for (int i = 0; i < RELAY_N; i++) {
        pinMode(RELAY_PIN_1 + i, OUTPUT);
        digitalWrite(RELAY_PIN_1 + i, LOW);
    }
}

// -------------------------------

void blink() {
      digitalWrite(LED_PIN, HIGH);
      delay(100 /*ms*/);
      digitalWrite(LED_PIN, LOW);
}

void trip_relay(int index /* 0..RELAY_N-1 */) {
    digitalWrite(RELAY_PIN_1 + index, HIGH);
    delay(100);
    digitalWrite(RELAY_PIN_1 + index, LOW);
}

bool read_input(int index) {
    return digitalRead(IN_PIN_1) == HIGH;
}

//#define SRCP_UNINIT 0
//#define SRCP_HANDSHAKE 1
//#define SRCP_INFO 2
//#define SRCP_COMMAND 3
//
//int srcp_mode = SRCP_UNINIT;
//
//void parse_srcp() {
//    String s(_buf_1024);
//    
//    _wifi.write((const uint8_t*)_buf_1024, 3);
//}

bool was_wifi_connected = false;

void process_wifi() {

    if (!was_wifi_connected) {
        if (_wifi.connected() == DIGIFI_CONNECTED) {
            const char *msg = "Connection started\n";
            Serial.print(msg);
            _wifi.write((const uint8_t*)msg, strlen(msg));
        }
    } else if (_wifi.available() == 0 || !_wifi.isRecentActivity()) {
        const char *msg = "Maybe disconnected\n";
        Serial.print(msg);
        _wifi.write((const uint8_t*)msg, strlen(msg));
    }
    

  
//      // Accepted commands are @01..16 to trip relay 1..16
//      // and @?A..Z to read sensor A..Z.
//
//    int n;
//    while (_wifi.available() > 0 && (n = _wifi.readBytes(_buf_1024, 1023)) > 0) {
//        if (n > 1023) { n = 1023; }
//        _buf_1024[n] = 0;
//        parse_srcp();
//        Serial.print("recv:_");
//        Serial.print(buf);
//        Serial.println("_");
//        if (n >= 2 && *buf == '@') {
//            buf++;
//            if (buf[0] == '?' && isAlpha(buf[1])) {
//                int n = buf[1] - 'A';
//                if (n >= 0 && n < IN_N) {
//                    bool state = read_input(n);
//                    buf[0] = state ? '+' : '-';
//                    blink();
//                    _wifi.write((const uint8_t*)_buf_1024, 3);
//                }
//            } else if (isDigit(buf[0]) && isDigit(buf[1])) {
//                int n = 10 * (buf[0] - '0') + (buf[1] - '0');
//                if (n >= 1 && n <= RELAY_N) {
//                    Serial.println("Trip relay");
//                    trip_relay(n - 1);
//                    blink();
//                    _wifi.write((const uint8_t*)_buf_1024, 3);
//                }
//            }
//        }
//    }
}

void loop() {
    if (_blink_ms < millis()) {
        _blink_ms = millis() + 1000;
        blink();

        // keep connected
        if (_wifi.isRecentActivity()) {
            // _wifi.write('.');
            blink();
        }
    }

    process_wifi();
}
