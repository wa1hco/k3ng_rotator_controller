// Pin assignments for native PC build.
// Motor and sensor pins use small "virtual" numbers so the HAL can tell
// them apart.  Everything else is 0 (ignored by stubs).

#define pins_h  // prevents rotator_pins.h from being included a second time

// Virtual pin numbers — must match rotator_hal_native.cpp PIN_* constants.
#define rotate_cw               1   // HAL: HIGH → simulator CW motion
#define rotate_ccw              2   // HAL: HIGH → simulator CCW motion
#define rotator_analog_az       3   // HAL: analogRead returns simulated ADC

#define rotate_cw_ccw           0
#define rotate_cw_pwm           0
#define rotate_ccw_pwm          0
#define rotate_cw_ccw_pwm       0
#define rotate_cw_freq          0
#define rotate_ccw_freq         0
#define button_cw               0
#define button_ccw              0
#define serial_led              0
#define azimuth_speed_voltage   0
#define overlap_led             0
#define brake_az                0
#define az_speed_pot            0
#define az_preset_pot           0
#define preset_start_button     0
#define button_stop             0
#define rotation_indication_pin 0
#define blink_led               0
#define az_stepper_motor_pulse  0
#define az_rotation_stall_detected 0

// LCD pins (unused — no display feature enabled)
#define lcd_4_bit_rs_pin        0
#define lcd_4_bit_enable_pin    0
#define lcd_4_bit_d4_pin        0
#define lcd_4_bit_d5_pin        0
#define lcd_4_bit_d6_pin        0
#define lcd_4_bit_d7_pin        0

#define heading_reading_inhibit_pin 0
#define pin_status_led          0
