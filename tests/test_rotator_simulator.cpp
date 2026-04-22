#include <cmath>

#include "rotator_simulator.h"
#include "test_framework.h"

namespace {

bool within_tolerance(float value, float target, float tolerance) {
  return std::fabs(value - target) <= tolerance;
}

void run_for(RotatorSimulator &sim, int milliseconds, int step_ms) {
  int elapsed = 0;
  while (elapsed < milliseconds) {
    const int slice = (milliseconds - elapsed > step_ms) ? step_ms : (milliseconds - elapsed);
    sim.step_ms(static_cast<uint32_t>(slice));
    elapsed += slice;
  }
}

}  // namespace

TEST_CASE(simulator_reaches_target_azimuth) {
  RotatorSimConfig config;
  config.max_speed_deg_per_sec = 60.0f;
  config.accel_deg_per_sec2 = 120.0f;
  config.tolerance_deg = 0.2f;

  RotatorSimulator sim(config);
  sim.set_current_azimuth(0.0f);
  sim.set_target_azimuth(90.0f);

  run_for(sim, 6000, 20);

  REQUIRE_TRUE(within_tolerance(sim.current_azimuth(), 90.0f, 0.25f));
  REQUIRE_EQ(RotatorMotionState::Idle, sim.motion_state());
}

TEST_CASE(simulator_uses_shortest_wraparound_path) {
  RotatorSimConfig config;
  config.max_speed_deg_per_sec = 30.0f;
  config.accel_deg_per_sec2 = 120.0f;
  config.tolerance_deg = 0.2f;

  RotatorSimulator sim(config);
  sim.set_current_azimuth(350.0f);
  sim.set_target_azimuth(10.0f);

  sim.step_ms(100);
  REQUIRE_EQ(RotatorMotionState::Clockwise, sim.motion_state());

  run_for(sim, 5000, 20);

  REQUIRE_TRUE(within_tolerance(sim.current_azimuth(), 10.0f, 0.25f));
  REQUIRE_EQ(RotatorMotionState::Idle, sim.motion_state());
}

TEST_CASE(simulator_can_report_noisy_sensor_values) {
  RotatorSimConfig config;
  config.sensor_noise_stddev_deg = 1.0f;

  RotatorSimulator sim(config);
  sim.set_current_azimuth(123.0f);

  const float reading = sim.read_azimuth_sensor();

  REQUIRE_FALSE(within_tolerance(reading, 123.0f, 0.0001f));
}

TEST_CASE(dual_axis_simulator_moves_both_axes_to_target) {
  DualAxisRotatorSimConfig config;
  config.azimuth.wraparound = true;
  config.elevation.wraparound = false;
  config.elevation.min_angle_deg = 0.0f;
  config.elevation.max_angle_deg = 180.0f;

  DualAxisRotatorSimulator sim(config);
  sim.set_current_azimuth(10.0f);
  sim.set_current_elevation(5.0f);
  sim.set_target_azimuth(120.0f);
  sim.set_target_elevation(45.0f);

  for (int i = 0; i < 500; ++i) {
    sim.step_ms(20);
  }

  REQUIRE_TRUE(within_tolerance(sim.current_azimuth(), 120.0f, 0.3f));
  REQUIRE_TRUE(within_tolerance(sim.current_elevation(), 45.0f, 0.3f));
}

TEST_CASE(stall_fault_prevents_axis_movement) {
  DualAxisRotatorSimConfig config;
  DualAxisRotatorSimulator sim(config);

  sim.set_current_azimuth(30.0f);
  sim.set_target_azimuth(120.0f);
  sim.set_fault_active(RotatorAxisId::Azimuth, RotatorFault::Stall, true);

  for (int i = 0; i < 200; ++i) {
    sim.step_ms(20);
  }

  REQUIRE_TRUE(within_tolerance(sim.current_azimuth(), 30.0f, 0.01f));
}

TEST_CASE(sensor_dropout_fault_returns_no_reading) {
  DualAxisRotatorSimConfig config;
  DualAxisRotatorSimulator sim(config);

  sim.set_fault_active(RotatorAxisId::Azimuth, RotatorFault::SensorDropout, true);
  const auto reading = sim.read_azimuth_sensor();

  REQUIRE_FALSE(reading.has_value());
}
