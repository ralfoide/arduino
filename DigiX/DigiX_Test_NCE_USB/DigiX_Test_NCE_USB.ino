#define LED_PIN         13

#define LOCO 5000

// --------------------------------

void blink() {
      digitalWrite(LED_PIN, HIGH);
      delay(100 /*ms*/);
      digitalWrite(LED_PIN, LOW);
      delay(100 /*ms*/);
}

// --------------------------------

void setup_pause() {
    // DigiX trick - since we are on serial over USB wait for character to be
    // entered in serial terminal
    // Delay start by up to 5 seconds unless a key is pressed.
    // This should give enough to reprogram a bad behaving program.
    Serial2.begin(9600); 
    for (int i = 0; i < 5 && !Serial.available(); i++) {
        Serial2.println("Enter any key to begin");
        blink();
        delay(1000);
    }
    Serial2.println("Starting");
}

void setup() {
    SerialUSB.begin(9600); 
    setup_pause();

}

void nce_protocol() {
 
  #define LOCO_H (0xC0 + (LOCO >> 8))
  #define LOCO_L (LOCO & 0xFF)
  
  // NCE USB 0xA2 <adr H/L> 07 10|00 (4=F0/light on/off), reply 1 byte status
  SerialUSB.write(0xA2);
  SerialUSB.write(LOCO_H);
  SerialUSB.write(LOCO_L);
  SerialUSB.write(0x07);
  SerialUSB.write(0x10);
  
  if (SerialUSB.read() == '!') {
    blink();
  } else {
    blink();
    blink();
  }
  delay(1000);

  SerialUSB.write(0xA2);
  SerialUSB.write(LOCO_H);
  SerialUSB.write(LOCO_L);
  SerialUSB.write(0x07);
  SerialUSB.write((byte)0x00);
  
  SerialUSB.read();
  
  if (SerialUSB.read() == '!') {
    blink();
  } else {
    blink();
    blink();
  }
  delay(1000);
}

int test_once = 0;
void loop() {
  // put your main code here, to run repeatedly:
  if (test_once == 0) {
    nce_protocol();
  } else {
    delay(2000);
    blink();
  }
  test_once++;
}
