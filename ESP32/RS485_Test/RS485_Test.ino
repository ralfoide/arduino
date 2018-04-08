/*
  RS485 TEST

  RS485 board, example: https://arduino-info.wikispaces.com/SoftwareSerialRS485Example
  Pins: 
    VCC (next to B) to ESP32 5V
    GND (next to A) to ESP32 GND
    A / B: RS485, 2 middle pins from a 6P6C or a 4P4C
    DI (driver in) to Serial TX (GPIO1)
    RO (receive out) to Serial RX (GPIO3)
    DE (driver enable) to GPIO2
    RE (receive enable) to GPIO2, connected to DE.
  Note:
    RE is inverted (active low), DE is active high.
    RE low / DE low ==> drivers are inactive, RO outputs the signal from A/B (receive).
    RE high / DE high ==> RO disabled, drivers are active and emit the signal from DI (transmit).

  NCE Cab Bus runs at 9600 bauds, 8 data bits, no parity, 2 stop bits.

  RS485: https://datasheets.maximintegrated.com/en/ds/MAX1487-MAX491.pdf
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

#define RS_TX_PIN     TX     // Transmit from ESP32 to RS485, Data
#define RS_RX_PIN     RX     // Receive from RS485 into ESP32, Data 
#define RS_ENABLE_PIN 2      // Receive from RS485 into ESP32 when low, transmit to RS485 when high
#define RS_ENABLE_RX  LOW
#define RS_ENABLE_TX  HIGH

// U8G2 INIT -- OLED U8G2 constructor for ESP32 WIFI_KIT_32 I2C bus
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

int u8g2_redraw = true;

void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

//---------------

int led_blink = 0;

void init_led() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void alternate_blink() {
  digitalWrite(LED_BUILTIN, led_blink ? LOW : HIGH);
  led_blink = !led_blink;
}

//---------------

int recv_buf[7*3];
int recv_index = -1;
int recv_len = sizeof(recv_buf) / sizeof(int);

void init_rs485() {
  pinMode(RS_ENABLE_PIN, OUTPUT);
  digitalWrite(RS_ENABLE_PIN, RS_ENABLE_RX);

  Serial.begin(9600, SERIAL_8N2);
  memset(recv_buf, 0, sizeof(recv_buf));
}

void check_rs485() {
  if (Serial.available()) {
    recv_buf[recv_index++] = Serial.read();
    if (recv_index == recv_len) {
      recv_index = 0;
    }
    u8g2_redraw = true;
  }
}


//---------------

char hexa[17] = "0123456789ABCDEF";
char draw_num[12];

void draw() {
  if (!u8g2_redraw) {
    return;
  }
  u8g2.clearBuffer();
  u8g2_prepare();
  // u8g2.setFont(u8g2_font_6x10_tf); -- from prepare
  // Font is 6x10, coords are x,y
  u8g2.drawStr(0, 0, "RS485 TEST");

  int co = 0;
  int li = 15;
  int k = recv_len - 1;
  for(int i = 0; i <= k; i++) {
    int b = recv_index + i;
    if (b >= recv_len) {
      b -= recv_len;
    }
    b = recv_buf[b];

    draw_num[0] = i == 0 ? '|' : ' ';
    draw_num[1] = hexa[(b & 0x0F0) >> 4];
    draw_num[2] = hexa[(b & 0x00F)];
    draw_num[3] = 0;

    if (co >= 120) {
      co = 0;
      li += 15;
    }
    u8g2.drawStr(co, li, draw_num);
    co += 18;
  }
  
  u8g2.sendBuffer();
  u8g2_redraw = false;
}

//---------------

void setup(void) {
  init_led();
  init_rs485();
  u8g2.begin();
}

void loop(void) {
  alternate_blink();
  draw();
  check_rs485();
}

