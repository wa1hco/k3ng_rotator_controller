#pragma once
#ifdef NATIVE_BUILD

#include <cstdint>
#include "rotator_simulator.h"

// Azimuth simulator instance — range derived from AZIMUTH_*_EEPROM_INITIALIZE
// defines in the user's rotator_settings file.
extern RotatorAxisSimulator g_az_sim;

// Advance the physics model by wall-clock time elapsed since last call.
void native_sim_step();

// Arduino stub implementations — wired to the simulator via the logical pin
// names in the user's rotator_pins file (rotate_cw, rotate_ccw, rotator_analog_az).
int  native_analog_read(uint8_t pin);
void native_digital_write(uint8_t pin, uint8_t val);

#endif // NATIVE_BUILD
