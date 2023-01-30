#a="""
from machine import I2C, Pin, PWM
from neopixel import NeoPixel
import ssd1306
import time

DO_NEO_DEMO = False
DO_SERVO_DEMO = True

# OLED display
rst = Pin(16, Pin.OUT)
rst.value(1)
scl = Pin(15, Pin.OUT, Pin.PULL_UP)
sda = Pin(4, Pin.OUT, Pin.PULL_UP)
i2c = I2C(scl=scl, sda=sda, freq=450000)
oled = ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3c)

# Pin 0 = PRG button on board.
# Button.value() is reversed. 1=released 0=pushed.
btn_pin = Pin(0, Pin.IN)

# Servo, min/max values for Blue Arrow S03611
servo_pwm = PWM(Pin(12), freq=50)
SERVO_MIN = 30  # -- measured min at 26
SERVO_MAX = 100 # -- measured max at 108
servo = SERVO_MIN
servo_pwm.duty(servo)

NLED   = 10
BR_MAX = 64  # max brightness
BR_LVL = 4   # number of brightness levels (including max)
MAX_INDEX = NLED + 2*BR_LVL
rgb_mask = 1
np_pin = Pin(13, Pin.OUT)
np = NeoPixel(np_pin, NLED)
np.fill( (0,0,0) )
np.write()

BAR="|/-\\\\"
BARlen = len(BAR)
counter = 1
def update_oled(msg = None):
    if not msg: msg = "MicroPython"
    oled.fill(0)
    oled.text("ESP32", 45, 5)
    oled.text(msg, 20, 20) 
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
    
while DO_NEO_DEMO:
    print(counter)
    update_oled()
    update_neopix()
    counter += 1
    if counter % MAX_INDEX == 0:
        rgb_mask = 1 + ((rgb_mask + 1) % 7)
        print("rgb_mask", rgb_mask)
    time.sleep(0.1)

if DO_SERVO_DEMO:
    update_oled("Use PRG btn") 
while DO_SERVO_DEMO:
    if not btn_pin.value():  # PRG button pressed
        print(str(counter) +  " srv " + str(servo) )
        update_oled("Servo " + str(servo))
        counter += 1
        servo_pwm.duty(servo)
        servo += 10
        if servo > SERVO_MAX: servo = SERVO_MIN
        time.sleep(0.25)

    time.sleep(0.1)
    
# end
#"""
#f=open("main.py", "w")
#f.write(a)
#f.close()
