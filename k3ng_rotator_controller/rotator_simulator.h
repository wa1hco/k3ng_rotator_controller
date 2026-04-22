#ifndef ROTATOR_SIMULATOR_H
#define ROTATOR_SIMULATOR_H

#include <cstdint>
#include <optional>
#include <random>

enum class RotatorMotionState {
  Idle,
  Clockwise,
  CounterClockwise,
};

enum class RotatorFault {
  Stall,
  SensorDropout,
  StuckClockwiseRelay,
  StuckCounterClockwiseRelay,
};

struct RotatorAxisConfig {
  bool wraparound = true;
  float min_angle_deg = 0.0f;
  float max_angle_deg = 360.0f;
  float max_speed_deg_per_sec = 30.0f;
  float accel_deg_per_sec2 = 60.0f;
  float tolerance_deg = 0.5f;
  float sensor_noise_stddev_deg = 0.0f;
  float backlash_deg = 0.0f;
  float stiction_speed_deg_per_sec = 0.0f;
};

struct RotatorSimConfig {
  float max_speed_deg_per_sec = 30.0f;
  float accel_deg_per_sec2 = 60.0f;
  float tolerance_deg = 0.5f;
  float sensor_noise_stddev_deg = 0.0f;
};

class RotatorAxisSimulator {
 public:
  explicit RotatorAxisSimulator(const RotatorAxisConfig &config = RotatorAxisConfig{});

  void set_target_angle(float angle_deg);
  void set_current_angle(float angle_deg);
  void set_max_speed(float max_speed_deg_per_sec);
  void set_acceleration(float accel_deg_per_sec2);
  void set_sensor_noise(float stddev_deg);
  void set_backlash(float backlash_deg);
  void set_stiction(float stiction_speed_deg_per_sec);

  void set_fault_active(RotatorFault fault, bool active);
  bool is_fault_active(RotatorFault fault) const;

  void step_ms(uint32_t delta_ms);

  float current_angle() const;
  float target_angle() const;
  float current_speed_deg_per_sec() const;
  RotatorMotionState motion_state() const;
  std::optional<float> read_sensor();

 private:
  float normalize_angle(float angle_deg) const;
  float shortest_signed_delta(float from_deg, float to_deg) const;
  float clamp_non_wrap_angle(float angle_deg) const;

  RotatorAxisConfig config_;
  float current_angle_deg_ = 0.0f;
  float target_angle_deg_ = 0.0f;
  float current_speed_deg_per_sec_ = 0.0f;
  RotatorMotionState motion_state_ = RotatorMotionState::Idle;
  float backlash_remaining_deg_ = 0.0f;
  int last_motion_direction_ = 0;

  bool fault_stall_ = false;
  bool fault_sensor_dropout_ = false;
  bool fault_stuck_cw_relay_ = false;
  bool fault_stuck_ccw_relay_ = false;

  std::mt19937 rng_;
};

struct DualAxisRotatorSimConfig {
  RotatorAxisConfig azimuth;
  RotatorAxisConfig elevation;
};

enum class RotatorAxisId {
  Azimuth,
  Elevation,
};

class DualAxisRotatorSimulator {
 public:
  explicit DualAxisRotatorSimulator(
      const DualAxisRotatorSimConfig &config = DualAxisRotatorSimConfig{});

  void step_ms(uint32_t delta_ms);

  void set_target_azimuth(float angle_deg);
  void set_target_elevation(float angle_deg);
  void set_current_azimuth(float angle_deg);
  void set_current_elevation(float angle_deg);

  float current_azimuth() const;
  float current_elevation() const;
  float target_azimuth() const;
  float target_elevation() const;

  RotatorMotionState azimuth_motion_state() const;
  RotatorMotionState elevation_motion_state() const;

  std::optional<float> read_azimuth_sensor();
  std::optional<float> read_elevation_sensor();

  void set_fault_active(RotatorAxisId axis, RotatorFault fault, bool active);
  bool is_fault_active(RotatorAxisId axis, RotatorFault fault) const;

  RotatorAxisSimulator &azimuth_axis();
  RotatorAxisSimulator &elevation_axis();
  const RotatorAxisSimulator &azimuth_axis() const;
  const RotatorAxisSimulator &elevation_axis() const;

 private:
  RotatorAxisSimulator azimuth_;
  RotatorAxisSimulator elevation_;
};

class RotatorSimulator {
 public:
  explicit RotatorSimulator(const RotatorSimConfig &config = RotatorSimConfig{});

  void set_target_azimuth(float azimuth_deg);
  void set_current_azimuth(float azimuth_deg);
  void set_max_speed(float max_speed_deg_per_sec);
  void set_acceleration(float accel_deg_per_sec2);
  void set_sensor_noise(float stddev_deg);

  void step_ms(uint32_t delta_ms);

  float current_azimuth() const;
  float target_azimuth() const;
  float current_speed_deg_per_sec() const;
  RotatorMotionState motion_state() const;

  // Simulated sensor reading with optional Gaussian noise.
  float read_azimuth_sensor();

  void set_fault_active(RotatorFault fault, bool active);
  bool is_fault_active(RotatorFault fault) const;

 private:
  RotatorAxisSimulator axis_;
};

#endif  // ROTATOR_SIMULATOR_H
