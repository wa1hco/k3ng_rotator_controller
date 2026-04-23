// rotator_features_hco_board.h
// Feature set for WA1HCO custom rotator board (Teensy 4.0).
//
// Native PC simulation status:
//   SIMULATED NOW : Yaesu protocol, azimuth position (single-ended pot approximation),
//                   motor start/stop, overlap logic, stop command
//   NOT YET SIMULATED : FEATURE_AZ_POSITION_POTENTIOMETER_DUAL_END (dual-end ADC,
//                       wiper noise cancellation), H-bridge motor direction control,
//                       front-panel buttons, preset pot
//
// For the native build, position sensing uses FEATURE_AZ_POSITION_POTENTIOMETER
// (top-end ADC only) as a stand-in until the dual-end HAL is implemented.

#define FEATURE_YAESU_EMULATION
#define OPTION_GS_232B_EMULATION

// Position sensor: switch to FEATURE_AZ_POSITION_POTENTIOMETER_DUAL_END once
// the dual-end HAL path is implemented (track 2).
#define FEATURE_AZ_POSITION_POTENTIOMETER

#define LANGUAGE_ENGLISH
