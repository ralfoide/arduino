#include <stdio.h>
#include <string.h>

// -----------------------------------------------

//typedef unsigned char   uint8_t;
//typedef unsigned int    uint16_t;
//typedef unsigned long   uint32_t;

//typedef char            boolean;
//typedef unsigned char   byte;
//typedef unsigned long   IPAddress;
//typedef unsigned long   size_t ;

// -----------

class String {
public:
};

class DigiFi {
public:
    void println(const char *s) {}
    void begin(int aBaud = 9600, bool en = false) {}
    void setDebug(bool debugStateVar) {}
    bool ready() { return true; }
    String server(uint16_t port) { return String(); }
    int available( void ) { return 0; }
    int readBytes(char *buf, size_t size) { return 0; }
    size_t write(const uint8_t *buf, size_t size) { return 0; }
};

// -----------
class _MockSerial {
public:
    static void print(const char *s) {}
    static void println(const char *s) {}
    static void println(String &s) {}

    static void begin(int aBaud = 9600) {}
    static size_t available() { return 0; }
/*
    size_t peek() { return 0; }
    size_t flush() { return 0; }
    size_t read() { return 0; }
    size_t read(uint8_t *buf, size_t size) { return 0; }
*/
};

_MockSerial Serial;

// -----------------------------------------------

#define LOW  0
#define HIGH 1
#define INPUT 0
#define INPUT_HIGH 1
#define OUTPUT 2

long _next_millis = 1000;
long millis() { return _next_millis; }

void delay(size_t ms) {}

void pinMode(int pin, int mode) {}
int  digitalRead(int pin) { return 0; }

char _digital_writes[1024];

void digitalWrite(int pin, int value) { 
    int n = strlen(_digital_writes);
    sprintf(_digital_writes + n, "%c%03d,",
        value == HIGH ? 'H' : (value == LOW ? 'L' : '?'),
        pin);
}

void _reset_read_writes() {
    *_digital_writes = 0;
}

// -----------------------------------------------

#define __TEST

static int _failures = 0;

void _expect(int line, 
        long expected, const char *exp_str, 
        long actual, const char *act_str, 
        const char *msg) {
    if (expected != actual) {
        printf("[%4d] Expected %s [%d], Actual %s [%d]%s%s\n", 
            line, 
            exp_str, expected, 
            act_str, actual, 
            msg == NULL ? "" : ": ",
            msg == NULL ? "" : msg);
        _failures++;
    }
}

void _expect(int line, 
        const char *expected, const char *exp_str, 
        const char *actual,   const char *act_str, 
        const char *msg) {
    if (strcmp(expected, actual) != 0) {
        printf("[%4d] Expected %s [%s], Actual %s [%s]%s%s\n", 
            line, 
            exp_str, expected, 
            act_str, actual, 
            msg == NULL ? "" : ": ",
            msg == NULL ? "" : msg);
        _failures++;
    }
}

#define EXPECT(a, b) _expect(__LINE__, a, # a, b, # b, NULL)
#define EXPECT_MSG(a, b, msg) _expect(__LINE__, a, # a, b, # b, msg)

#include "LayoutWifi.ino"

void test_blink() {
    printf("- TEST: blink\n");
    _reset_read_writes();
    blink();
    EXPECT("H013,L013,", _digital_writes);
}

void test_trip_relay() {
    printf("- TEST: trip_relay\n");
    _reset_read_writes();
    trip_relay(0);
    trip_relay(RELAY_N - 1);
    EXPECT("H090,L090,H097,L097,", _digital_writes);
}

int main(int argc, char **argv) {
    printf("- Start\n");
    EXPECT_MSG(1, argc, "No program arguments accepted.");

    test_blink();
    test_trip_relay();
    
    printf("- End. %d failures%s\n",
        _failures,
        _failures == 0 ? " => SUCCESS." : "");
    return 0;
}
