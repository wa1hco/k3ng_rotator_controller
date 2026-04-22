// Settings for native PC test build — reuse defaults then override for test convenience.
#include "rotator_settings.h"

// Accept movement commands immediately; no point waiting 5 seconds in a test environment.
#undef  ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS
#define ROTATIONAL_AND_CONFIGURATION_CMD_IGNORE_TIME_MS 0
