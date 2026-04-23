// rotator_settings_hco_board.h
// Settings for WA1HCO custom rotator board.
// Includes K3NG defaults then overrides for this hardware.

#include "rotator_settings.h"

// HCO board uses a standard 0-360° rotator (no overlap zone).
#undef  AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE
#define AZIMUTH_STARTING_POINT_EEPROM_INITIALIZE        0

#undef  AZIMUTH_ROTATION_CAPABILITY_EEPROM_INITIALIZE
#define AZIMUTH_ROTATION_CAPABILITY_EEPROM_INITIALIZE   360

// Tighter stopping tolerance — dual-end ADC gives better position accuracy.
#undef  AZIMUTH_TOLERANCE
#define AZIMUTH_TOLERANCE                               2.0

// Accept commands immediately in test builds.
#undef  ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS
#define ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS 0

// Simulated rotator mechanics — set to match your physical rotator.
// HCO board targets a typical TV rotator: ~30 seconds for full 360° sweep.
#define NATIVE_SIMULATOR_MAX_SPEED_DEG_PER_SEC          12.0   // 360/30s
#define NATIVE_SIMULATOR_ACCEL_DEG_PER_SEC2             6.0    // ~2s ramp
