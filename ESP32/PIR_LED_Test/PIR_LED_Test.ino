/*
  PIR + LED TEST

  PIR: Clone of HC-SR501 
  Pins: 
    PIR VCC to ESP32 5V (do not use 3.3V, not enough power!)
    PIR GND to ESP32 GND
    PIR OUT to GPIO input 36  ==> Output is 3.3V (PIR module has a 7133 voltage regulator)
  LEDs:
    1k Ohm res to GND
    Green LED between res and GPIO 13
    Orange LED between res and GPIO 12
    Red LED between res and GPIO 14

  U8G2: https://github.com/olikraus/u8g2/wiki
*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);


void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

//---------------

#define PIR_IN 36

int led_count = 0;
int led_pin[] = { 13, 12, 14 };
int led_num = sizeof(led_pin) / sizeof(int);
int led_off = 0;
int led_on = 0;
int led_blink = 0;
int pir_state = led_num - 1;
int pir_presence = 0;
int pir_alert = 0;

void init_led() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < led_num; i++) {
    pinMode(led_pin[i], OUTPUT);
    digitalWrite(led_pin[i], LOW);
  }
}

void led_rgb_scan(int count_delay) {
  if (++led_count < count_delay) {
    return;
  }
  led_count = 0;

  digitalWrite(led_pin[led_off], LOW);    // turn the LED off
  digitalWrite(led_pin[led_on ], HIGH);   // turn the LED on
  led_off = led_on;
  led_on++;
  if (led_on >= led_num) {
    led_on = 0;
  }
}

void led_rgb_set(int index) {
  digitalWrite(led_pin[led_off], LOW);    // turn the LED off
  led_off = led_on;
  led_on = index;
  digitalWrite(led_pin[led_on ], HIGH);   // turn the LED on
}

void alternate_blink() {
  digitalWrite(LED_BUILTIN, led_blink ? LOW : HIGH);
  led_blink = !led_blink;
}

void init_pir() {
  pinMode(PIR_IN, INPUT);
}

void check_pir() {
  int pir_on = digitalRead(PIR_IN) == HIGH;
  digitalWrite(LED_BUILTIN, pir_on ? HIGH : LOW);
  if (pir_on) {
    pir_state = 0;
  } else {
    pir_state++;
  }
  pir_presence = pir_state < 30;
  pir_alert    = pir_state < 10;
  led_rgb_set(pir_alert ? 0 : (pir_presence ? 1 : 2));
}

char draw_num[12];

void draw() {
  int str_len;
  u8g2_prepare();
  // u8g2.setFont(u8g2_font_6x10_tf); -- from prepare
  // Font is 6x10, coords are x,y
  u8g2.drawStr(0, 0, "PIR TEST");
  
  u8g2.drawStr(0, 15, "Presence:");
  itoa(pir_state, draw_num, 10);
  u8g2.drawStr(60, 15, draw_num);

  // u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.setFont(u8g2_font_t0_22b_tf);
  str_len = u8g2.drawStr(20, 40, pir_alert ? "MOTION" : (pir_presence ? "Presence" : "Idle"));
  if (pir_alert || pir_presence) {
    u8g2.drawRFrame(10, 36, 84-66+str_len, 26, 3);
  }
}

//---------------

void setup(void) {
  init_led();
  init_pir();
  u8g2.begin();
}

void loop(void) {
  // picture loop  
  u8g2.clearBuffer();
  draw();
  u8g2.sendBuffer();

  check_pir();

  // delay
  delay(100 /*ms*/);
}

