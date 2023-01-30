from machine import Pin
import time

led_pin1 = Pin(25, Pin.OUT)
led_pin2 = Pin(21, Pin.OUT)

led_pin1.off()
led_pin2.off()


def blink(pin):
    pin.on()
    time.sleep_ms(250)
    pin.off()

def main_loop():
    n = 0
    while True:
        blink(led_pin1)
        time.sleep(1) # 1 sec

        blink(led_pin2)
        time.sleep(1) # 1 sec

        n += 1
        print("blink",n)


print("Main is ", __name__)
if __name__ == "__main__":
    main_loop()
