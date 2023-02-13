/*
  Adafruit VL53L0X TimeOfFlight Sensor

  I2C pins SDA=21 + SCL=22

  U8G2: https://github.com/olikraus/u8g2/wiki
  (Arduino IDE > Sketch > Libraries > U8G2)

  VL53L0X: https://github.com/adafruit/Adafruit_VL53L0X
  (Arduino IDE > Sketch > Libraries > Adafruit VL53L0X)
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_VL53L0X.h>
#include <Wire.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*SLC*/ 15, /*SDA*/ 4, /*RESET*/ 16);


void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

//---------------

#define PIN_LED2 19
#define THRESHOLD_MM 150
Adafruit_VL53L0X tof;
VL53L0X_RangingMeasurementData_t tof_measure;
uint16_t tof_dist_mm = 0;

void init_led() {
  Serial.println("@@ Init LED");
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
}

void blink_led() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(250 /*ms*/);
  digitalWrite(LED_BUILTIN, HIGH);
}

void init_tof() {
  Serial.println("@@ Init TOF");
  Wire.begin(/*SDA*/ 21, /*SLC*/ 22);
  if (!tof.begin()) {
    Serial.println(F("@@ VL53L0X begin failed"));
    while (true) {
      blink_led();
      delay(250);
    }
  }
}

void loop_tof() {
  Serial.println("@@ Loop TOP");
  tof.rangingTest(&tof_measure, /*debug*/ true);
  tof_dist_mm = tof_measure.RangeMilliMeter;
  
  if (tof_measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("@@ TOF distance mm: "); 
    Serial.println(tof_dist_mm);

    digitalWrite(PIN_LED2, tof_dist_mm < THRESHOLD_MM ? HIGH : LOW);

  } else {
    Serial.println("@@ TOF out of range ");
    digitalWrite(PIN_LED2, LOW);
  }
}

char draw_num[12];
int y_offset = 0;
#define YTXT 22

void draw() {
  int str_len;

  int y = abs(y_offset - 8);
  
  u8g2_prepare();
  // u8g2.setFont(u8g2_font_6x10_tf); //-- from prepare
  // Font is 6x10, coords are x,y
  u8g2.setFont(u8g2_font_t0_22b_tf);

  u8g2.drawStr(0, y, "VL53L0X TEST");
  y += YTXT;
  
  String dt = String(tof_dist_mm) + " mm";
  u8g2.drawStr(0, y, dt.c_str());
  y += YTXT;

  // Frame is an empty Box. Box is filled.
  u8g2.drawFrame(0, y, 128, 8);
  float w = (128.0f / 2000.0f) * tof_dist_mm;
  u8g2.drawBox(0, y, min(128, max(0, (int)w)) , 8);
  


  /*
  u8g2.drawStr(0, 15, "Presence:");
  itoa(pir_state, draw_num, 10);
  u8g2.drawStr(60, 15, draw_num);

  // u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.setFont(u8g2_font_t0_22b_tf);
  str_len = u8g2.drawStr(20, 40, pir_alert ? "MOTION" : (pir_presence ? "Presence" : "Idle"));
  if (pir_alert || pir_presence) {
    u8g2.drawRFrame(10, 36, 84-66+str_len, 26, 3);
  }
  */

  y_offset = (y_offset + 1) % 16;
}

//---------------

void setup(void) {
  Serial.begin(115200);
  init_led();
  u8g2.begin();
  init_tof();
}

void loop(void) {
  blink_led();
  loop_tof();
  
  // picture loop  
  u8g2.clearBuffer();
  draw();
  u8g2.sendBuffer();

  // delay is done by the blinking led
  // delay(100 /*ms*/);
}

