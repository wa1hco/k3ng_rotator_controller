// Minimal feature set for native PC compilation and testing.
// Goal: smallest possible dependency surface — one protocol, one position sensor,
// no display, no elevation, no clock, no remote.
// Expand incrementally as each path is verified.

#define FEATURE_YAESU_EMULATION
#define FEATURE_AZ_POSITION_POTENTIOMETER

#define LANGUAGE_ENGLISH
#define OPTION_GS_232B_EMULATION
