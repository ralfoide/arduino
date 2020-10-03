from machine import I2C, Pin
from neopixel import NeoPixel
import ssd1306
import time

# OLED display
rst = Pin(16, Pin.OUT)
rst.value(1)
scl = Pin(15, Pin.OUT, Pin.PULL_UP)
sda = Pin(4, Pin.OUT, Pin.PULL_UP)
i2c = I2C(scl=scl, sda=sda, freq=450000)
oled = ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3c)



NLED   = 300
BR_MAX = 64 # max brightness
BR_LVL = 4   # number of brightness levels (including max)
MAX_INDEX = NLED + 2*BR_LVL
rgb_mask = 1
np_pin = Pin(13, Pin.OUT)
np = NeoPixel(np_pin, NLED)
np.fill( (0,0,0) )
np.write()

BAR="|/-\\"
BARlen = len(BAR)
counter = 1
def update_oled():
    oled.fill(0)
    oled.text('ESP32', 45, 5)
    oled.text('MicroPython', 20, 20) 
    oled.text(BAR[counter % BARlen] + " " + str(counter), 45, 35)
    oled.show()

def set_np_led(index, brightness, distance):
    distance = abs(distance)
    c = 0
    if distance < BR_LVL:
        k = (BR_LVL - distance) / float(BR_LVL)
        c = int(brightness * k * k)
    # print(index, c)
    np[index] = ((rgb_mask & 1) and c or 0, (rgb_mask & 2) and c or 0, (rgb_mask & 4) and c or 0)
    
def update_neopix():
    # Using 10 LEDs: counter%10 is the "current" LED
    # set it to the "max" intensity. degrade nearby ones by 1/4th so 4 max
    curr = counter % (MAX_INDEX) - BR_LVL
    for i in range(0, NLED):
        set_np_led(i, BR_MAX, curr - i)
    np.write()
    
while True:
    print(counter)
    update_oled()
    update_neopix()
    counter += 1
    if counter % MAX_INDEX == 0:
        rgb_mask = 1 + ((rgb_mask + 1) % 7)
        print("rgb_mask", rgb_mask)
    time.sleep(0.01)

# end
