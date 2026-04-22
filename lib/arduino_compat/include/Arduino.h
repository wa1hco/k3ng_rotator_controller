#pragma once
#ifndef ARDUINO_H
#define ARDUINO_H

// Standard headers first — must precede any min/max/abs definitions
#include <stdint.h>
#include <stddef.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <cstring>

// ── Basic types ───────────────────────────────────────────────────────────────
typedef uint8_t  byte;
typedef uint16_t word;
typedef bool     boolean;

// ── Constants ────────────────────────────────────────────────────────────────
#define HIGH  1
#define LOW   0
#define INPUT         0
#define OUTPUT        1
#define INPUT_PULLUP  2
#define INPUT_PULLDOWN 3

#define PI          3.14159265358979323846f
#define TWO_PI      6.28318530717958647692f
#define DEG_TO_RAD  0.01745329251994329577f
#define RAD_TO_DEG  57.2957795130823208768f
#define EULER       2.71828182845904523536f

// ── Bit macros ────────────────────────────────────────────────────────────────
#define bitRead(v,b)    (((v)>>(b))&0x01)
#define bitSet(v,b)     ((v)|=(1UL<<(b)))
#define bitClear(v,b)   ((v)&=~(1UL<<(b)))
#define bitToggle(v,b)  ((v)^=(1UL<<(b)))
#define bit(b)          (1UL<<(b))
#define lowByte(w)      ((uint8_t)((w)&0xFF))
#define highByte(w)     ((uint8_t)(((w)>>8)&0xFF))

// ── Math ─────────────────────────────────────────────────────────────────────
// Use std versions — macros conflict with C++ standard library template overloads.
using std::min;
using std::max;
using std::abs;

#define constrain(v,lo,hi)  ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
#define sq(x)               ((x)*(x))
#define radians(deg)        ((deg)*DEG_TO_RAD)
#define degrees(rad)        ((rad)*RAD_TO_DEG)

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ── Timing ───────────────────────────────────────────────────────────────────
inline unsigned long millis() {
    using namespace std::chrono;
    static const auto kStart = steady_clock::now();
    return (unsigned long)duration_cast<milliseconds>(steady_clock::now()-kStart).count();
}

inline unsigned long micros() {
    using namespace std::chrono;
    static const auto kStart = steady_clock::now();
    return (unsigned long)duration_cast<microseconds>(steady_clock::now()-kStart).count();
}

// Non-blocking on PC — K3NG's main loop uses millis() for timing; delay() only
// appears in setup() splash screens and similar non-critical paths.
inline void delay(unsigned long /*ms*/) {}
inline void delayMicroseconds(unsigned int /*us*/) {}

// ── Analog pin aliases (AVR: A0=14, A1=15, …) ────────────────────────────────
#define A0  14
#define A1  15
#define A2  16
#define A3  17
#define A4  18
#define A5  19
#define A6  20
#define A7  21

// ── AVR SP register (stack pointer) — not meaningful on PC ───────────────────
#ifndef SP
#define SP  ((uint16_t)0)
#endif

// ── GPIO ─────────────────────────────────────────────────────────────────────
inline void    pinMode(uint8_t /*pin*/, uint8_t /*mode*/) {}
inline void    analogWrite(uint8_t /*pin*/, int /*val*/) {}
inline void    analogReference(uint8_t /*mode*/) {}
inline int     digitalRead(uint8_t /*pin*/) { return LOW; }

#ifdef NATIVE_BUILD
// Implemented in rotator_hal_native.cpp — routes sensor/motor pins to simulator.
int  native_analog_read(uint8_t pin);
void native_digital_write(uint8_t pin, uint8_t val);
inline int  analogRead(uint8_t pin) { return native_analog_read(pin); }
inline void digitalWrite(uint8_t pin, uint8_t val) { native_digital_write(pin, val); }
#else
inline int  analogRead(uint8_t /*pin*/) { return 512; }
inline void digitalWrite(uint8_t /*pin*/, uint8_t /*val*/) {}
#endif

// ── Interrupts ────────────────────────────────────────────────────────────────
#define CHANGE   1
#define FALLING  2
#define RISING   3
inline void attachInterrupt(uint8_t /*irq*/, void(*)(), int /*mode*/) {}
inline void detachInterrupt(uint8_t /*irq*/) {}
inline void interrupts()   {}
inline void noInterrupts() {}

// ── Tone ─────────────────────────────────────────────────────────────────────
inline void tone(uint8_t /*pin*/, unsigned int /*freq*/, unsigned long /*dur*/=0) {}
inline void noTone(uint8_t /*pin*/) {}

// ── Pulse ─────────────────────────────────────────────────────────────────────
inline unsigned long pulseIn(uint8_t, uint8_t, unsigned long timeout=1000000UL) { return 0; }

// ── String and Serial ─────────────────────────────────────────────────────────
#include "WString.h"
#include "HardwareSerial.h"

// ── Random ───────────────────────────────────────────────────────────────────
#include <cstdlib>
inline void  randomSeed(unsigned long seed) { srand((unsigned)seed); }
inline long  random(long max)               { return rand() % max; }
inline long  random(long min, long max)     { return min + rand() % (max-min); }

// ── Misc ──────────────────────────────────────────────────────────────────────
inline void yield() {}

// ── AVR binary literal macros (B00000001 etc.) ───────────────────────────────
// Provides Bxxxxxxxx constants (0..255) as the Arduino IDE does.
// We define the macro form so arbitrary 8-bit patterns work.
#define B00000000 0x00
#define B00000001 0x01
#define B00000010 0x02
#define B00000011 0x03
#define B00000100 0x04
#define B00000101 0x05
#define B00000110 0x06
#define B00000111 0x07
#define B00001000 0x08
#define B00001001 0x09
#define B00001010 0x0A
#define B00001011 0x0B
#define B00001100 0x0C
#define B00001101 0x0D
#define B00001110 0x0E
#define B00001111 0x0F
#define B00010000 0x10
#define B00100000 0x20
#define B00110000 0x30
#define B01000000 0x40
#define B01100000 0x60
#define B10000000 0x80
#define B11000000 0xC0
#define B11111111 0xFF
// For any other B-literal not listed here, C++14 binary literals work: 0b00000001

// ── Flash string helper ───────────────────────────────────────────────────────
// On PC, F("string") is just a const char* — no flash/RAM distinction.
class __FlashStringHelper;
#undef  F
#define F(s) (reinterpret_cast<const __FlashStringHelper *>(s))

// pgm_read_byte needed before avr/pgmspace.h is visible in some paths
#ifndef pgm_read_byte
#include "avr/pgmspace.h"
#endif

// ── dtostrf — AVR stdlib function, missing on PC ──────────────────────────────
#include <cstdio>
inline char *dtostrf(double val, signed char width, unsigned char prec, char *s) {
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%%d.%df", (int)width, (int)prec);
    snprintf(s, 32, fmt, val);
    return s;
}

#endif // ARDUINO_H
