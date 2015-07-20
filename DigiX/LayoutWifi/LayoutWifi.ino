#include "RalfDigiFi.h"
#include "SRCPServerSerial1.h"
#include "SRCPDeviceManager.h"
#include "FBSwitchSensor.h"
#include "GATurnout.h"
#include "Logger.h"

#define LED_PIN     13
#define RELAY_PIN_1 90
#define RELAY_N      8

#define SENSOR_POLLING  500     // Sensor polling interval (in millis I guess)

DigiFi _wifi;

int _relay = 0;
unsigned long _relay_ms = 0;
char _recv_buf_1024[1024];
srcp::SRCPServerSerial1 _server;

// --------------------------------
void setup() {
  setup_pause();
  setup_wifi();
  setup_relays();
  setup_srcp();
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

void setup_relays() {
  // initialize digital pin 90-97 as an output.
  for (int i = 0; i < RELAY_N; i++) {
    pinMode(RELAY_PIN_1 + i, OUTPUT);
    digitalWrite(RELAY_PIN_1 + i, LOW);
  }
}

dev::FBSwitchSensor _fb1(1, A0, A3);
dev::GATurnout _sw1(1, RELAY_PIN_1 + 0, RELAY_PIN_1 + 1);

void setup_srcp() {
  DeviceManager.addFeedback(&_fb1);
  DeviceManager.addAccessoire(&_sw1);
  _server.begin(9600);
  INFO ( "Server listen " );
}

// -------------------------------

void blink() {
  digitalWrite(LED_PIN, HIGH);
  delay(100 /*ms*/);
  digitalWrite(LED_PIN, LOW);
}

void __trip_relay(int index /* 0..RELAY_N-1 */) {
  digitalWrite(RELAY_PIN_1 + index, HIGH);
  delay(100);
  digitalWrite(RELAY_PIN_1 + index, LOW);
}

void __process_relay() {
  unsigned long now_ms = millis();
  if (_relay_ms <= now_ms) {
    _relay_ms = now_ms + 1000; // 1 second
    
    for (int i = 0; i < RELAY_N; i++) {
      digitalWrite(RELAY_PIN_1 + i, LOW);
      if (_relay == i) {
        __trip_relay(i);
      }
    }
    _relay++;
    if (_relay == RELAY_N) {
      _relay = 0;
    }
  }
}

void __process_wifi_test() {
  if (_wifi.serverRequest()){
    Serial.print("Request for: ");
    Serial.println(_wifi.serverRequestPath());
    if (_wifi.serverRequestPath() != "/") {
      _wifi.serverResponse("404 Not Found",404); 
    } else {
       _wifi.serverResponse("<html><body><h1>This is a test</h1></body></html>"); //defaults to 200
    }
  }
}

void __process_wifi() {
  if (Serial1.available()){
    int i;
    for (i = 0; i < 1023 && Serial1.available(); i++) {
      char c = Serial1.read();
      _recv_buf_1024[i] = c;
      if (c == '\n') {
        break;
      }
    }
    _recv_buf_1024[i+1] = 0;
    Serial.println(_recv_buf_1024);
    Serial1.print(_recv_buf_1024);
    if (i > 0) {
      char c = _recv_buf_1024[0];
      if (c >= '1' && c <= '8') {
        __trip_relay(c - '1');
      }
    }
  }
}

void loop() {
  blink();

  //--process_wifi();
  //--process_relay();
  _server.dispatch( SENSOR_POLLING );
  DeviceManager.refresh();

  delay(10 /*ms*/);
}
