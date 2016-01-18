#include <Wire.h> // from Sketch > Include Libraries > Contributed DigiX > Wire

// Sketch copied from https://digistump.com/board/index.php/topic,1411.msg6530.html#msg6530
//
// Comment from https://digistump.com/board/index.php/topic,1411.msg6534.html#msg6534
// indicate this won't work on a Due/DigiX.

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
    for (int i = 0; i < 5 && !Serial.available(); i++) {
        //--Serial.println("Enter any key to begin");
        blink();
        delay(1000);
    }
//    SerialUSB.println("Starting");
}

void setup_i2c() {
  Wire.begin();
}

void setup() {
  setup_pause();
  setup_i2c();
  SerialUSB.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices;

  SerialUSB.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    blink();
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      SerialUSB.print("I2C device found at address 0x");
      if (address<16) {
        SerialUSB.print("0");
      }
      SerialUSB.print(address, HEX);
      SerialUSB.println("  !");

      nDevices++;
    }
    else if (error==4) {
      SerialUSB.print("Unknow error at address 0x");
      if (address<16) {
        SerialUSB.print("0");
      }
      SerialUSB.println(address, HEX);
    }    
  }
  if (nDevices == 0) {
    SerialUSB.println("No I2C devices found\n");
  } else {
    SerialUSB.println("done\n");
  }

  delay(5000);           // wait 5 seconds for next scan
}
