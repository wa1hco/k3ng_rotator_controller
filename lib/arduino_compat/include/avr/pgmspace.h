#pragma once
// AVR PROGMEM stubs — on PC, flash and RAM are the same address space.
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define PROGMEM
#define PGM_P           const char *
#define PGM_VOID_P      const void *
#define PSTR(s)         (s)
// F() is already defined in Arduino.h using __FlashStringHelper; don't redefine.
#ifndef F
#define F(s)            (s)
#endif

#define pgm_read_byte(addr)   (*(const uint8_t  *)(addr))
#define pgm_read_word(addr)   (*(const uint16_t *)(addr))
#define pgm_read_dword(addr)  (*(const uint32_t *)(addr))
#define pgm_read_float(addr)  (*(const float    *)(addr))
#define pgm_read_ptr(addr)    (*(const void * const *)(addr))

#define strlen_P    strlen
#define strcpy_P    strcpy
#define strncpy_P   strncpy
#define strcat_P    strcat
#define strcmp_P    strcmp
#define strncmp_P   strncmp
#define sprintf_P   sprintf
#define printf_P    printf
#define memcpy_P    memcpy
