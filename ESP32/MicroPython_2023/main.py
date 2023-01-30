import esp
import machine
from machine import Pin, SoftI2C
import ssd1306
import time
import _thread

led_pin1 = Pin(25, Pin.OUT)
led_pin2 = Pin(21, Pin.OUT)

led_pin1.off()
led_pin2.off()


def blink(pin):
    pin.on()
    time.sleep_ms(250)
    pin.off()

def thread_led(pin):
    print("Thread LED", pin)
    while True:
        blink(pin)
        time.sleep_ms(1000)

def init_oled():
    global oled
    rst = Pin(16, Pin.OUT)
    rst.value(1)
    scl = Pin(15, Pin.OUT, Pin.PULL_UP)
    sda = Pin(4, Pin.OUT, Pin.PULL_UP)
    i2c = SoftI2C(scl=scl, sda=sda, freq=450000)
    oled = ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3c)

oled_y_offset = 0
oled_msg = "MicroPython"
def update_oled(msg = None):
    global oled_y_offset, oled_msg
    if msg: oled_msg = msg
    oled.fill(0)
    oled.text("ESP32", 45, 5 + oled_y_offset)
    oled.text(oled_msg, 20, 20 + oled_y_offset) 
    oled.show()
    oled_y_offset = (oled_y_offset + 1) % (64 - 8)

def main_loop():
    t_led1 = _thread.start_new_thread(thread_led, (led_pin1, ))
    time.sleep_ms(500)
    t_led2 = _thread.start_new_thread(thread_led, (led_pin2, ))
    time.sleep_ms(250)
    n = 0
    while True:
        update_oled()
        n += 1
        time.sleep_ms(1000)
        print("loop",n)


print("Main is ", __name__)
if __name__ == "__main__":
    print("CPU freq:", machine.freq() / 1000 / 1000, "MHz")
    print("Flash:", esp.flash_size() / 1024 / 1024, "MB")
    print("Start:", esp.flash_user_start() / 1024 / 1024, "MB")
    init_oled()
    main_loop()
