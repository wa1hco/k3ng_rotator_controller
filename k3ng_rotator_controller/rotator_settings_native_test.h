// Settings for native PC test build — reuse defaults then override for test convenience.
#include "rotator_settings.h"

// Accept movement commands immediately; no point waiting 5 seconds in a test environment.
#undef  ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS
#define ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS 0

// Simulated rotator speed — not a K3NG setting, it's a property of the physical
// rotator being modeled.  Set these to match your actual hardware.
// Defaults give a ~15 second full-sweep, fast enough for practical test runs.
#define NATIVE_SIMULATOR_MAX_SPEED_DEG_PER_SEC  30.0
#define NATIVE_SIMULATOR_ACCEL_DEG_PER_SEC2     30.0
