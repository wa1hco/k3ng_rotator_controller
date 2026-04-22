#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include "WString.h"

// Forward declaration — full definition is in Arduino.h where F() is also defined.
class __FlashStringHelper;

// Bases for print(n, base)
#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class HardwareSerial {
public:
    HardwareSerial() { setvbuf(stdout, nullptr, _IOLBF, 0); }
    void begin(unsigned long /*baud*/) {}
    void end() {}

    // On PC, Serial reads from stdin (fd=0) via raw POSIX I/O.
    // std::cin has internal buffering that causes available() / select()
    // to disagree — avoid it entirely for the receive path.
    int  available() {
        if (eof_) return 0;
        if (has_peek_) return 1;
        fd_set fds; FD_ZERO(&fds); FD_SET(0, &fds);
        struct timeval tv = {0, 0};
        if (select(1, &fds, nullptr, nullptr, &tv) <= 0) return 0;
        uint8_t b; ssize_t n = ::read(0, &b, 1);
        if (n <= 0) { eof_ = true; return 0; }
        peek_byte_ = b; has_peek_ = true;
        return 1;
    }
    int  read() {
        if (eof_) return -1;
        if (has_peek_) { has_peek_ = false; return peek_byte_; }
        uint8_t b; ssize_t n = ::read(0, &b, 1);
        if (n <= 0) { eof_ = true; return -1; }
        return (int)b;
    }
    int  peek() {
        if (eof_) return -1;
        if (has_peek_) return peek_byte_;
        fd_set fds; FD_ZERO(&fds); FD_SET(0, &fds);
        struct timeval tv = {0, 0};
        if (select(1, &fds, nullptr, nullptr, &tv) <= 0) return -1;
        uint8_t b; ssize_t n = ::read(0, &b, 1);
        if (n <= 0) { eof_ = true; return -1; }
        peek_byte_ = b; has_peek_ = true;
        return peek_byte_;
    }
    void flush()     { std::cout.flush(); }

    size_t write(uint8_t b)                      { std::cout.put((char)b); return 1; }
    size_t write(const char *s)                  { std::cout<<s; return strlen(s); }
    size_t write(const uint8_t *buf, size_t len) { for(size_t i=0;i<len;i++) std::cout.put((char)buf[i]); return len; }

    void print(const char *s)             { std::cout<<s; }
    void print(const String &s)           { std::cout<<s.c_str(); }
    void print(const __FlashStringHelper *s) { std::cout<<reinterpret_cast<const char*>(s); }
    void print(char c)                    { std::cout<<c; }
    void print(int n, int base=DEC)       { if(base==HEX) std::cout<<std::hex<<n<<std::dec; else std::cout<<n; }
    void print(unsigned int n, int base=DEC)   { std::cout<<n; }
    void print(long n, int base=DEC)      { std::cout<<n; }
    void print(unsigned long n, int base=DEC)  { std::cout<<n; }
    void print(float n, int dec=2)        { std::cout<<n; }
    void print(double n, int dec=2)       { std::cout<<n; }

    void println()                        { std::cout<<'\n'; }
    void println(const char *s)           { std::cout<<s<<'\n'; }
    void println(const String &s)         { std::cout<<s.c_str()<<'\n'; }
    void println(const __FlashStringHelper *s) { std::cout<<reinterpret_cast<const char*>(s)<<'\n'; }
    void println(char c)                  { std::cout<<c<<'\n'; }
    void println(int n, int base=DEC)     { print(n,base); std::cout<<'\n'; }
    void println(unsigned int n, int b=DEC)    { print(n,b); std::cout<<'\n'; }
    void println(long n, int base=DEC)    { print(n,base); std::cout<<'\n'; }
    void println(unsigned long n, int b=DEC)   { print(n,b); std::cout<<'\n'; }
    void println(float n, int dec=2)      { print(n,dec); std::cout<<'\n'; }
    void println(double n, int dec=2)     { print(n,dec); std::cout<<'\n'; }

    operator bool() { return true; }
private:
    bool    eof_      = false;
    bool    has_peek_ = false;
    uint8_t peek_byte_= 0;
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;

// Alias used by some boards
typedef HardwareSerial Serial_;
typedef HardwareSerial usb_serial_class;
typedef HardwareSerial USBSerial;
