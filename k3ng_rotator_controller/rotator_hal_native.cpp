#ifdef NATIVE_BUILD

#include "rotator_hal_native.h"
#include "Arduino.h"  // for millis(), HIGH/LOW

#ifdef HARDWARE_HCO_BOARD
  #include "rotator_features_hco_board.h"
  #include "rotator_pins_hco_board.h"
  #include "rotator_settings_hco_board.h"
#else  // default: HARDWARE_NATIVE_TEST
  #include "rotator_features_native_test.h"
  #include "rotator_pins_native_test.h"
  #include "rotator_settings_native_test.h"
#endif

#include <algorithm>
#include <cmath>

// ── Simulator instance ────────────────────────────────────────────────────────
// Range is derived from K3NG's EEPROM-init settings so the simulator
// automatically matches whatever the user's rotator_settings file declares.
// Speed is a property of the physical rotator, not K3NG firmware — set it
// in rotator_settings_native_test.h via NATIVE_SIMULATOR_* defines.

static RotatorAxisConfig az_config = {
    /* wraparound           */ false,
    /* min_angle_deg        */ (float)AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE,
    /* max_angle_deg        */ (float)(AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE +
                                       AZIMUTH_ROTATION_CAPABILITY_EEPROM_INITIALIZE),
    /* max_speed_deg_per_sec*/ (float)NATIVE_SIMULATOR_MAX_SPEED_DEG_PER_SEC,
    /* accel_deg_per_sec2   */ (float)NATIVE_SIMULATOR_ACCEL_DEG_PER_SEC2,
    /* tolerance_deg        */ 0.5f,
    /* sensor_noise_stddev  */ 0.0f,
    /* backlash_deg         */ 0.0f,
    /* stiction             */ 0.0f,
};

RotatorAxisSimulator g_az_sim{az_config};

// ── ADC scaling helpers ───────────────────────────────────────────────────────
// K3NG reads: raw_az = float_map(adc, ccw_adc=1, cw_adc=1023, start, end)
// Inverse:    adc    = 1 + (angle - start) / range * 1022
static constexpr float AZ_CCW_DEG = (float)AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE;
static constexpr float AZ_CW_DEG  = (float)(AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE +
                                             AZIMUTH_ROTATION_CAPABILITY_EEPROM_INITIALIZE);
static constexpr int   ADC_CCW    = 1;
static constexpr int   ADC_CW     = 1023;

static int angle_to_adc(float angle_deg) {
    float t = (angle_deg - AZ_CCW_DEG) / (AZ_CW_DEG - AZ_CCW_DEG);
    t = std::max(0.0f, std::min(1.0f, t));
    return (int)std::roundf(ADC_CCW + t * (ADC_CW - ADC_CCW));
}

// ── native_sim_step ───────────────────────────────────────────────────────────
void native_sim_step() {
    static unsigned long last_ms = 0;
    unsigned long now = millis();
    if (last_ms == 0) { last_ms = now; return; }
    uint32_t delta = (uint32_t)(now - last_ms);
    last_ms = now;
    if (delta > 0) g_az_sim.step_ms(delta);
}

// ── native_analog_read ────────────────────────────────────────────────────────
// Uses rotator_analog_az from the user's rotator_pins file directly —
// no separate HAL pin constant needed.
int native_analog_read(uint8_t pin) {
    if (pin == rotator_analog_az) {
        native_sim_step();
        return angle_to_adc(g_az_sim.current_angle());
    }
    return 512;  // midpoint for any other pin
}

// ── native_digital_write ──────────────────────────────────────────────────────
// Uses rotate_cw / rotate_ccw from the user's rotator_pins file directly.
// K3NG asserts one pin HIGH and deasserts the other on each motor activation;
// evaluate both together to determine simulator target.
static uint8_t s_cw_pin_val  = LOW;
static uint8_t s_ccw_pin_val = LOW;

void native_digital_write(uint8_t pin, uint8_t val) {
    if      (pin == rotate_cw)  s_cw_pin_val  = val;
    else if (pin == rotate_ccw) s_ccw_pin_val = val;
    else return;

    if (s_cw_pin_val == HIGH && s_ccw_pin_val != HIGH) {
        g_az_sim.set_target_angle(AZ_CW_DEG);
    } else if (s_ccw_pin_val == HIGH && s_cw_pin_val != HIGH) {
        g_az_sim.set_target_angle(AZ_CCW_DEG);
    } else if (s_cw_pin_val == LOW && s_ccw_pin_val == LOW) {
        g_az_sim.set_target_angle(g_az_sim.current_angle());
    }
}

#endif // NATIVE_BUILD
