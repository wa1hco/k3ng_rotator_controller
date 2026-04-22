#include <string>

#include "rotator_protocol_shim.h"
#include "rotator_simulator.h"
#include "test_framework.h"

TEST_CASE(protocol_shim_supports_set_and_read_commands) {
  DualAxisRotatorSimConfig config;
  DualAxisRotatorSimulator sim(config);
  RotatorProtocolShim shim(&sim);

  REQUIRE_EQ(std::string("OK"), shim.handle_command("M180"));

  for (int i = 0; i < 300; ++i) {
    sim.step_ms(20);
  }

  const std::string response = shim.handle_command("C");
  REQUIRE_TRUE(response.rfind("AZ ", 0) == 0);
}

TEST_CASE(protocol_shim_supports_dual_axis_w_command) {
  DualAxisRotatorSimConfig config;
  DualAxisRotatorSimulator sim(config);
  RotatorProtocolShim shim(&sim);

  REQUIRE_EQ(std::string("OK"), shim.handle_command("W 200 50"));

  for (int i = 0; i < 500; ++i) {
    sim.step_ms(20);
  }

  shim.poll_display();
  const ControllerDisplayState display = shim.display_state();

  REQUIRE_TRUE(display.azimuth_readout_deg.has_value());
  REQUIRE_TRUE(display.elevation_readout_deg.has_value());
  REQUIRE_TRUE(display.azimuth_readout_deg.value() > 150.0f);
  REQUIRE_TRUE(display.elevation_readout_deg.value() > 30.0f);
}

TEST_CASE(protocol_shim_reports_sensor_dropout) {
  DualAxisRotatorSimConfig config;
  DualAxisRotatorSimulator sim(config);
  RotatorProtocolShim shim(&sim);

  sim.set_fault_active(RotatorAxisId::Azimuth, RotatorFault::SensorDropout, true);
  const std::string response = shim.handle_command("C");

  REQUIRE_TRUE(response.find("---.-") != std::string::npos);
}
