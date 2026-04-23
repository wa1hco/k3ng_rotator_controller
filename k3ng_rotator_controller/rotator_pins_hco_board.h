// rotator_pins_hco_board.h
// Pin assignments for WA1HCO custom rotator board, Teensy 4.0.
//
// Motor drive: L298N H-bridge
//   MOTOR_AZ_PIN (20) = ENA  — high to run motor
//   IN1Pin       (17) = IN1  — high = CW direction
//
// Position sensor: dual-end potentiometer with grounded wiper
//   AZ_POS_TOP_PIN (14 / A0) — top end of pot
//   AZ_POS_BOT_PIN (15 / A1) — bottom end of pot
//
// Native simulation notes:
//   rotate_cw  is mapped to MOTOR_AZ_PIN so the HAL sees motor activation.
//   rotate_ccw is mapped to IN1Pin  (low = CCW direction when motor runs).
//   rotator_analog_az is mapped to AZ_POS_TOP_PIN (top-end only until dual-end
//   HAL is implemented).  These mappings approximate the H-bridge behavior
//   well enough for closed-loop position testing.

#define pins_h  // prevents rotator_pins.h from being included a second time

// ── Physical board pins ───────────────────────────────────────────────────────
#define MOTOR_AZ_PIN     20   // L298N ENA — motor enable
#define IN1Pin           17   // L298N IN1 — direction (high = CW)
#define BRAKE_AZ_PIN     16   // high to release brake
#define AZ_POS_TOP_PIN   14   // A0 — top of pot
#define AZ_POS_BOT_PIN   15   // A1 — bottom of pot
#define AZ_PRESET_PIN    18   // A4 — preset pot wiper
#define BUTTON_CW_PIN     7
#define BUTTON_CCW_PIN    6
#define TeensyLED        13

// ── K3NG logical pin names ────────────────────────────────────────────────────
#define rotate_cw               MOTOR_AZ_PIN   // motor enable (HAL: triggers CW sim)
#define rotate_ccw              IN1Pin          // direction low = CCW (HAL: triggers CCW sim)
#define rotate_cw_pwm           0
#define rotate_ccw_pwm          0
#define rotate_cw_ccw_pwm       0
#define rotate_cw_freq          0
#define rotate_ccw_freq         0

#define rotator_analog_az       AZ_POS_TOP_PIN  // top-end ADC (stand-in for dual-end)
#define azimuth_speed_voltage   0
#define brake_az                BRAKE_AZ_PIN
#define az_speed_pot            0
#define az_preset_pot           AZ_PRESET_PIN
#define preset_start_button     0
#define button_stop             0
#define button_cw               BUTTON_CW_PIN
#define button_ccw              BUTTON_CCW_PIN
#define serial_led              TeensyLED
#define overlap_led             0
#define blink_led               0
#define rotation_indication_pin 0
#define az_rotation_stall_detected 0
#define rotate_cw_ccw           0
#define heading_reading_inhibit_pin 0
#define pin_status_led          0

// ── LCD (not fitted on HCO board) ────────────────────────────────────────────
#define lcd_4_bit_rs_pin        0
#define lcd_4_bit_enable_pin    0
#define lcd_4_bit_d4_pin        0
#define lcd_4_bit_d5_pin        0
#define lcd_4_bit_d6_pin        0
#define lcd_4_bit_d7_pin        0
