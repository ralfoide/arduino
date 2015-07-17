#define USB_CFG_DEVICE_NAME     'D','i','g','i','T','h','r','o','t'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>

#define LED     1    // on-board led on digital pin 1
#define ADC1    1    // ADC channel 1
#define ADC1_P2 2    // ADC1 is connected to digital pin 2

byte in = 0;
int value = 0;  // ADC value is 10-bits
char buf[12];   // "-2147483648\0" = 12 characters.

void setup() {
  DigiUSB.begin();
  pinMode(LED,     OUTPUT);
  pinMode(ADC1_P2, INPUT);
  blink();
  digitalWrite(LED, LOW);
}

void blink() {
  digitalWrite(LED, LOW);
  delay(50 /*ms*/);
  digitalWrite(LED, HIGH);
  delay(240 /*ms*/);
  digitalWrite(LED, LOW);
}

void loop() {
  DigiUSB.refresh();
  if (DigiUSB.available() > 0) {
    in = DigiUSB.read();
    if (in == 't') {
      trigger();
    } else if (in == 'b') {
      blink();
    }
  }
}

void trigger() {
  // Read 10-bit ADC value and output to usb
  // Take average of 4 reads
  // 10-bits = 1024.
  value  = analogRead(ADC1);
  value += analogRead(ADC1);
  value += analogRead(ADC1);
  value += analogRead(ADC1);
  value /= 4;
  buf[5] = 0;
  buf[4] = '0'+ (value % 10);    // 123x
  value /= 10;
  buf[3] = '0'+ (value % 10);    // 12x4
  value /= 10;
  buf[2] = '0'+ (value % 10);    // 1x34
  value /= 10;
  buf[1] = '0'+ (value % 10);    // x234
  value /= 10;
  buf[0] = '0'+ (value % 10);    // x1234 -- should be zero
  DigiUSB.println(buf);

}

