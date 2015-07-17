/*
All this does is:
- blip led for 5 seconds
- close relay for 0.5 seconds
- blip led for 2 seconds
- done, nothing else.

Used to turn on portal when power comes back.
No USB connection (USB brick is used to power it.)
*/


#define USB_CFG_DEVICE_NAME     'D','i','g','i','R','e','l','a','y'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>

#define LED   1
#define RELAY 5

#define MS_LED    200
#define MS_RELAY 1000

int led() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  return 100;
}

int relay() {
  digitalWrite(RELAY, HIGH);    // close relay
  delay(MS_RELAY);
  digitalWrite(RELAY, LOW);     // open
  return MS_RELAY;
}

void setup() {
  DigiUSB.begin();
  // Initialize the relay pin as output
  pinMode(LED,   OUTPUT);   
  pinMode(RELAY, OUTPUT);   
  digitalWrite(LED,   LOW);
  digitalWrite(RELAY, LOW);     // Turn the relay off (open)
 
  for (int i = 0; i < 5; i++) {
    delay(1000 - led());
  }

  relay();

}

void loop() {
  // blip led every 10 minutes
  delay(1000 * 60 * 10 - led());
}

