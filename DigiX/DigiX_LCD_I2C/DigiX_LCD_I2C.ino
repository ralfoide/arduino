#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x3F for a 20 chars and 4 lines display

// --------------------------------

#define LED_PIN         13

void blink() {
      digitalWrite(LED_PIN, HIGH);
      delay(100 /*ms*/);
      digitalWrite(LED_PIN, LOW);
}

// --------------------------------

void setup_pause() {
    // DigiX trick - since we are on serial over USB wait for character to be
    // entered in serial terminal
    // Delay start by up to 5 seconds unless a key is pressed.
    // This should give enough to reprogram a bad behaving program.
    SerialUSB.begin(9600); 
    for (int i = 0; i < 5 && !SerialUSB.available(); i++) {
        SerialUSB.println("Enter any key to begin");
        blink();
        delay(1000);
    }
    SerialUSB.println("Starting");
}

void helloworld() {
  SerialUSB.println("LCD init");
  lcd.init();
  // Print a message to the LCD.
  SerialUSB.println("LCD backlight");
  lcd.display();
  lcd.backlight();
  SerialUSB.println("Line 0");
  blink();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  lcd.setCursor(0,1);
  lcd.print("Line 2");
  lcd.setCursor(0,2);
  lcd.print("Line 3");
  lcd.setCursor(0,3);
  lcd.print("Uptime:");
}

void setup() {
  setup_pause();

  helloworld();
}

void loop() {
  long mil = millis();
  int sec = mil / 1000;
  int dec = (mil / 100) % 10;
  String s(sec);
  lcd.setCursor(20 - 2 - s.length(), 3);
  lcd.print(s);
  lcd.print('.');
  lcd.print(dec);
  
  delay(250);
  blink();
}
