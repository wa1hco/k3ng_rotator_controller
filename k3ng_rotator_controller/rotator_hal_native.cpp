#ifdef NATIVE_BUILD

#include "rotator_hal_native.h"
#include "Arduino.h"  // for millis(), HIGH/LOW

#include <algorithm>
#include <cmath>
#include <cstdio>   // for fprintf(stderr,...) tracing

// ── Simulator instance ────────────────────────────────────────────────────────
// Range matches K3NG's default EEPROM: azimuth_starting_point=180°,
// azimuth_rotation_capability=450° → physical range [180°, 630°].
static RotatorAxisConfig az_config = {
    /* wraparound           */ false,
    /* min_angle_deg        */ 180.0f,
    /* max_angle_deg        */ 630.0f,
    /* max_speed_deg_per_sec*/ 30.0f,  // ~15 seconds for full 450° sweep
    /* accel_deg_per_sec2   */ 30.0f,
    /* tolerance_deg        */ 0.5f,
    /* sensor_noise_stddev  */ 0.0f,
    /* backlash_deg         */ 0.0f,
    /* stiction             */ 0.0f,
};

RotatorAxisSimulator g_az_sim{az_config};

// ── ADC scaling helpers ───────────────────────────────────────────────────────
// K3NG reads: raw_az = float_map(adc, ccw_adc=1, cw_adc=1023, start=180, end=630)
// Inverse:    adc    = 1 + (angle - 180) / 450 * 1022
static constexpr float AZ_CCW_DEG = 180.0f;
static constexpr float AZ_CW_DEG  = 630.0f;
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
    if (last_ms == 0) {
        last_ms = now;
        return;
    }
    uint32_t delta = (uint32_t)(now - last_ms);
    last_ms = now;
    if (delta > 0) {
        g_az_sim.step_ms(delta);
    }
}

// ── native_analog_read ────────────────────────────────────────────────────────
// Advances the physics model to the current wall-clock time before returning
// the sensor value — lazy evaluation, no need to poll from main().
int native_analog_read(uint8_t pin) {
    if (pin == PIN_AZ_POT) {
        native_sim_step();
        int adc = angle_to_adc(g_az_sim.current_angle());
        return adc;
    }
    return 512;  // midpoint for any other pin
}

// ── native_digital_write ──────────────────────────────────────────────────────
// K3NG asserts rotate_cw or rotate_ccw HIGH to start rotation, and deasserts
// the *other* direction pin (sets it LOW) at the same time.  We must look at
// both pins together to decide the simulator target:
//   CW  HIGH, CCW LOW  → moving CW  → target = max_angle
//   CW  LOW,  CCW HIGH → moving CCW → target = min_angle
//   CW  LOW,  CCW LOW  → stopped    → target = current_angle
static uint8_t s_cw_pin_val  = LOW;
static uint8_t s_ccw_pin_val = LOW;

void native_digital_write(uint8_t pin, uint8_t val) {
    if (pin == PIN_ROTATE_CW)        s_cw_pin_val  = val;
    else if (pin == PIN_ROTATE_CCW)  s_ccw_pin_val = val;
    else return;  // all other pins: no-op

    // Re-evaluate target after any motor pin change.
    if (s_cw_pin_val == HIGH && s_ccw_pin_val != HIGH) {
        g_az_sim.set_target_angle(AZ_CW_DEG);
    } else if (s_ccw_pin_val == HIGH && s_cw_pin_val != HIGH) {
        g_az_sim.set_target_angle(AZ_CCW_DEG);
    } else if (s_cw_pin_val == LOW && s_ccw_pin_val == LOW) {
        g_az_sim.set_target_angle(g_az_sim.current_angle());
    }

}

#endif // NATIVE_BUILD
