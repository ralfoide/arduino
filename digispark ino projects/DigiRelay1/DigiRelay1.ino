#define USB_CFG_DEVICE_NAME     'D','i','g','i','R','e','l','a','y'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>
byte in = 0;
int next = 0;

void setup() {
    DigiUSB.begin();
    // Initialize the relay pin as output
    pinMode(5, OUTPUT);   
    digitalWrite(5, LOW);     // Turn the relay off (open)
    DigiUSB.println("Relay demo: O=open/off, 1|c=close/on");
}

void loop() {
  DigiUSB.refresh();
  if (DigiUSB.available() > 0) {
    in = DigiUSB.read();
       
    if (in == 'o' || in == 'O' || in == '0') {
      digitalWrite(5, LOW);     // Turn the relay off (open)
      DigiUSB.println("Relay: O=open/off");
    } else if (in == 'c' || in == 'C' || in == '1') {
      digitalWrite(5, HIGH);    // Turn the relay on (close)
      DigiUSB.println("Relay: 1,c=close/on");
    }
  }
}

