/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */

#define RELAY1 90
#define RELAYN 8

int relay = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 90-97 as an output.
  for (int i = 0; i < RELAYN; i++) {
    pinMode(RELAY1 + i, OUTPUT);
    digitalWrite(RELAY1 + i, LOW);
  }
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(150);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(150);              // wait for a second
  }

  for (int i = 0; i < RELAYN; i++) {
    digitalWrite(RELAY1 + i, LOW);
    if (relay == i) {
      digitalWrite(RELAY1 + i, HIGH);
      delay(100);
      digitalWrite(RELAY1 + i, LOW);
    }
  }
  relay++;
  if (relay == RELAYN) {
    relay = 0;
  }

  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}
