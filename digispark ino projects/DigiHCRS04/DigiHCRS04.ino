#define USB_CFG_DEVICE_NAME     'D','i','g','i','H','C','R','S','4'
#define USB_CFG_DEVICE_NAME_LEN 9
#include <DigiUSB.h>

#define SOUND_SPEED_MPS 340 // m/s
#define TRIG 0
#define ECHO 1
#define LED  2
#define DEBUG 0

byte in = 0;
int n;
char buf[12]; // "-2147483648\0" = 12 characters.
unsigned long start_us, stop_us, result_mm, avg_mm;
#if DEBUG
unsigned long dbg_read_us;
#endif

void setup() {
  DigiUSB.begin();
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(TRIG, LOW);
  digitalWrite(LED, LOW);
}

void loop() {
  DigiUSB.refresh();
  if (DigiUSB.available() > 0) {
    in = DigiUSB.read();
    if (in == '1') {
      trigger(); 
      DigiUSB.println(ultoa(result_mm, buf, 10));
    } else if (in == '3') {
      avg3();
      DigiUSB.println(ultoa(avg_mm, buf, 10));
    } else if (isAlphaNumeric(in)) {
      DigiUSB.println("#HC-SR04. 1=trigger once, 3=trigger 3 avg");
    }
  }
}

// performs 3 triggers, result in millimetres in global avg_mm
void avg3() {
    trigger();
    avg_mm = result_mm;

    for (n = 0; n < 20; n++) { 
      DigiUSB.refresh(); 
      delay(10 /*ms*/);
    }

    trigger();
    avg_mm += result_mm;

    for (n = 0; n < 20; n++) { 
      DigiUSB.refresh(); 
      delay(10 /*ms*/);
    }

    trigger();
    avg_mm += result_mm;
    avg_mm /= 3;

    for (n = 0; n < 20; n++) { 
      DigiUSB.refresh(); 
      delay(10 /*ms*/);
    }
}

// performs 1 trigger, result in millimetres in global result_mm
void trigger() {
  // turn led on
  digitalWrite(LED, HIGH);

  // trigger
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

#if DEBUG
  dbg_read_us = micros();
#endif

  // wait for echo to start... takes about 400-1000 us
  start_us = micros();
  while (digitalRead(ECHO) == LOW) {
    start_us = micros();
  }

  // measure pulse high  
  stop_us = micros();
  while (digitalRead(ECHO) == HIGH && (stop_us - start_us) < 60000) {
    stop_us = micros();
  }
  // pulse length
  // distance microseconds / 2 * 340 m/s / 1e6 (us) * 1000 (mm) to get it in millimetres.
  // divide by half since it's an echo (going there and back)
  result_mm = stop_us - start_us;
  result_mm = (result_mm * SOUND_SPEED_MPS) / 2000;

  // delay to make sure we don't fire another trigger too soon, wait 1/10th
  //--delay(100 /*ms*/);
  digitalWrite(LED, LOW);

#if DEBUG
  DigiUSB.print("Read to start: ");
  DigiUSB.println(ultoa(start_us - dbg_read_us, buf, 10));
  DigiUSB.print("Start to stop: ");
  DigiUSB.println(ultoa(stop_us - start_us, buf, 10));
#endif

//  return result_mm;
}

