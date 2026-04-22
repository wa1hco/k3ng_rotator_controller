#pragma once
#ifdef NATIVE_BUILD

#include <cstdint>
#include "rotator_simulator.h"

// Virtual pin numbers — must match rotator_pins_native_test.h.
static constexpr uint8_t PIN_ROTATE_CW  = 1;
static constexpr uint8_t PIN_ROTATE_CCW = 2;
static constexpr uint8_t PIN_AZ_POT     = 3;

// Azimuth simulator — 180° (CCW stop) to 630° (CW stop), non-wraparound.
// Matches K3NG's default EEPROM: starting_point=180, rotation_capability=450.
extern RotatorAxisSimulator g_az_sim;

// Called each loop iteration from native_k3ng.cpp to advance the physics model.
void native_sim_step();

// Arduino stub implementations — called from analogRead() / digitalWrite() in
// Arduino.h when NATIVE_BUILD is defined.
int  native_analog_read(uint8_t pin);
void native_digital_write(uint8_t pin, uint8_t val);

#endif // NATIVE_BUILD
