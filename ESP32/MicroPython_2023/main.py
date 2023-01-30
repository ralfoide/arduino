import esp
import machine
from machine import Pin
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

def main_loop():
    t_led1 = _thread.start_new_thread(thread_led, (led_pin1, ))
    time.sleep_ms(500)
    t_led2 = _thread.start_new_thread(thread_led, (led_pin2, ))
    time.sleep_ms(250)
    n = 0
    while True:
        n += 1
        time.sleep_ms(1000)
        print("loop",n)


print("Main is ", __name__)
if __name__ == "__main__":
    print("CPU freq:", machine.freq() / 1000 / 1000, "MHz")
    print("Flash:", esp.flash_size() / 1024 / 1024, "MB")
    print("Start:", esp.flash_user_start() / 1024 / 1024, "MB")
    main_loop()
