// Native PC build entry point.
// PlatformIO's native platform does not process .ino files, so we include the
// sketch here as a plain C++ translation unit.
// Excluded from MCU builds via build_src_filter in platformio.ini.
#ifdef NATIVE_BUILD
#include "rotator_hal_native.h"
// Forward declarations auto-generated from k3ng_rotator_controller.ino by
// extra_scripts/gen_ino_declarations.py (mimics Arduino IDE's prototype injection).
#include "rotator_native_declarations.h"
#include "k3ng_rotator_controller.ino"
// main() is defined at the end of k3ng_rotator_controller.ino under NATIVE_BUILD.
// native_sim_step() is called lazily from native_analog_read() each time K3NG
// samples the azimuth pot, so no separate call is needed here.
#endif
