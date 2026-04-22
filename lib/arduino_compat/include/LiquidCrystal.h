#pragma once
#include <cstdint>

// PC stub — K3NGdisplay already buffers everything; these calls are no-ops.
class LiquidCrystal {
public:
    LiquidCrystal(int,int,int,int,int,int) {}
    void begin(int, int)         {}
    void clear()                 {}
    void home()                  {}
    void noCursor()              {}
    void cursor()                {}
    void noBlink()               {}
    void blink()                 {}
    void display()               {}
    void noDisplay()             {}
    void setCursor(int, int)     {}
    void print(char)             {}
    void print(const char *)     {}
    void print(int)              {}
    void print(float)            {}
    size_t write(uint8_t)        { return 1; }
};
