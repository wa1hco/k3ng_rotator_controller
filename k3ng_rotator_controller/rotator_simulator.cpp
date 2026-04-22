#include "rotator_simulator.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kMinAcceleration = 0.01f;
constexpr float kMinSpeed = 0.001f;

float approach(float value, float target, float delta) {
  if (value < target) {
    return std::min(value + delta, target);
  }
  return std::max(value - delta, target);
}

float clampf(float value, float low, float high) {
  return std::max(low, std::min(high, value));
}

}  // namespace

RotatorAxisSimulator::RotatorAxisSimulator(const RotatorAxisConfig &config)
    : config_(config), rng_(0x4B334E47u) {
  if (config_.accel_deg_per_sec2 < kMinAcceleration) {
    config_.accel_deg_per_sec2 = kMinAcceleration;
  }
  if (config_.max_speed_deg_per_sec < 0.0f) {
    config_.max_speed_deg_per_sec = 0.0f;
  }
  if (config_.tolerance_deg < 0.0f) {
    config_.tolerance_deg = 0.0f;
  }
  if (config_.sensor_noise_stddev_deg < 0.0f) {
    config_.sensor_noise_stddev_deg = 0.0f;
  }
  if (config_.backlash_deg < 0.0f) {
    config_.backlash_deg = 0.0f;
  }
  if (config_.stiction_speed_deg_per_sec < 0.0f) {
    config_.stiction_speed_deg_per_sec = 0.0f;
  }

  current_angle_deg_ = normalize_angle(current_angle_deg_);
  target_angle_deg_ = current_angle_deg_;
}

void RotatorAxisSimulator::set_target_angle(float angle_deg) {
  target_angle_deg_ = normalize_angle(angle_deg);
}

void RotatorAxisSimulator::set_current_angle(float angle_deg) {
  current_angle_deg_ = normalize_angle(angle_deg);
  if (!config_.wraparound) {
    target_angle_deg_ = clamp_non_wrap_angle(target_angle_deg_);
  }
}

void RotatorAxisSimulator::set_max_speed(float max_speed_deg_per_sec) {
  config_.max_speed_deg_per_sec = std::max(0.0f, max_speed_deg_per_sec);
}

void RotatorAxisSimulator::set_acceleration(float accel_deg_per_sec2) {
  config_.accel_deg_per_sec2 = std::max(kMinAcceleration, accel_deg_per_sec2);
}

void RotatorAxisSimulator::set_sensor_noise(float stddev_deg) {
  config_.sensor_noise_stddev_deg = std::max(0.0f, stddev_deg);
}

void RotatorAxisSimulator::set_backlash(float backlash_deg) {
  config_.backlash_deg = std::max(0.0f, backlash_deg);
}

void RotatorAxisSimulator::set_stiction(float stiction_speed_deg_per_sec) {
  config_.stiction_speed_deg_per_sec = std::max(0.0f, stiction_speed_deg_per_sec);
}

void RotatorAxisSimulator::set_fault_active(RotatorFault fault, bool active) {
  switch (fault) {
    case RotatorFault::Stall:
      fault_stall_ = active;
      break;
    case RotatorFault::SensorDropout:
      fault_sensor_dropout_ = active;
      break;
    case RotatorFault::StuckClockwiseRelay:
      fault_stuck_cw_relay_ = active;
      break;
    case RotatorFault::StuckCounterClockwiseRelay:
      fault_stuck_ccw_relay_ = active;
      break;
  }
}

bool RotatorAxisSimulator::is_fault_active(RotatorFault fault) const {
  switch (fault) {
    case RotatorFault::Stall:
      return fault_stall_;
    case RotatorFault::SensorDropout:
      return fault_sensor_dropout_;
    case RotatorFault::StuckClockwiseRelay:
      return fault_stuck_cw_relay_;
    case RotatorFault::StuckCounterClockwiseRelay:
      return fault_stuck_ccw_relay_;
  }
  return false;
}

void RotatorAxisSimulator::step_ms(uint32_t delta_ms) {
  if (delta_ms == 0) {
    return;
  }

  if (!config_.wraparound) {
    current_angle_deg_ = clamp_non_wrap_angle(current_angle_deg_);
    target_angle_deg_ = clamp_non_wrap_angle(target_angle_deg_);
  }

  const float dt_sec = static_cast<float>(delta_ms) / 1000.0f;
  const float delta_before = shortest_signed_delta(current_angle_deg_, target_angle_deg_);

  if ((std::fabs(delta_before) <= config_.tolerance_deg &&
       std::fabs(current_speed_deg_per_sec_) < 0.5f) &&
      !fault_stuck_cw_relay_ && !fault_stuck_ccw_relay_) {
    current_angle_deg_ = target_angle_deg_;
    current_speed_deg_per_sec_ = 0.0f;
    motion_state_ = RotatorMotionState::Idle;
    return;
  }

  const float direction = (delta_before >= 0.0f) ? 1.0f : -1.0f;
  const float distance_to_stop = std::max(0.0f, std::fabs(delta_before) - config_.tolerance_deg);
  const float max_speed_for_stop =
      std::sqrt(2.0f * config_.accel_deg_per_sec2 * distance_to_stop);
  const float command_speed_mag = std::min(config_.max_speed_deg_per_sec, max_speed_for_stop);
  float command_speed = direction * command_speed_mag;

  if (fault_stuck_cw_relay_ && !fault_stuck_ccw_relay_) {
    command_speed = std::max(command_speed, config_.max_speed_deg_per_sec * 0.8f);
  }
  if (fault_stuck_ccw_relay_ && !fault_stuck_cw_relay_) {
    command_speed = std::min(command_speed, -config_.max_speed_deg_per_sec * 0.8f);
  }

  const float max_dv = config_.accel_deg_per_sec2 * dt_sec;
  current_speed_deg_per_sec_ = approach(current_speed_deg_per_sec_, command_speed, max_dv);

  if (std::fabs(command_speed) < 0.1f && std::fabs(delta_before) <= config_.tolerance_deg * 2.0f) {
    current_speed_deg_per_sec_ = approach(current_speed_deg_per_sec_, 0.0f, max_dv);
  }

  if (std::fabs(current_speed_deg_per_sec_) < config_.stiction_speed_deg_per_sec) {
    current_speed_deg_per_sec_ = 0.0f;
  }

  int direction_sign = 0;
  if (current_speed_deg_per_sec_ > 0.0f) {
    direction_sign = 1;
  } else if (current_speed_deg_per_sec_ < 0.0f) {
    direction_sign = -1;
  }

  if (direction_sign != 0 && last_motion_direction_ != 0 && direction_sign != last_motion_direction_) {
    backlash_remaining_deg_ = config_.backlash_deg;
  }

  if (fault_stall_) {
    direction_sign = 0;
    current_speed_deg_per_sec_ = 0.0f;
  }

  const float previous_angle = current_angle_deg_;
  float commanded_move_deg = current_speed_deg_per_sec_ * dt_sec;

  if (backlash_remaining_deg_ > 0.0f && std::fabs(commanded_move_deg) > 0.0f) {
    const float absorb = std::min(backlash_remaining_deg_, std::fabs(commanded_move_deg));
    backlash_remaining_deg_ -= absorb;
    commanded_move_deg = (std::fabs(commanded_move_deg) - absorb) * ((commanded_move_deg >= 0.0f) ? 1.0f : -1.0f);
  }

  current_angle_deg_ = normalize_angle(current_angle_deg_ + commanded_move_deg);

  const float delta_after = shortest_signed_delta(current_angle_deg_, target_angle_deg_);
  // >= / <= so delta_before==0 (target set to exact current position) counts as
  // a crossing when the rotor is still moving through that point.
  const bool crossed_target =
      ((delta_before >= 0.0f && delta_after < 0.0f) || (delta_before <= 0.0f && delta_after > 0.0f));
  const bool tiny_move =
      std::fabs(shortest_signed_delta(previous_angle, current_angle_deg_)) < config_.tolerance_deg;

  if (!fault_stuck_cw_relay_ && !fault_stuck_ccw_relay_ &&
      (crossed_target || (tiny_move && std::fabs(delta_after) <= config_.tolerance_deg))) {
    current_angle_deg_ = target_angle_deg_;
    current_speed_deg_per_sec_ = 0.0f;
    motion_state_ = RotatorMotionState::Idle;
    return;
  }

  if (!config_.wraparound) {
    const float clamped = clamp_non_wrap_angle(current_angle_deg_);
    if (clamped != current_angle_deg_) {
      current_angle_deg_ = clamped;
      current_speed_deg_per_sec_ = 0.0f;
    }
  }

  if (std::fabs(current_speed_deg_per_sec_) < kMinSpeed) {
    current_speed_deg_per_sec_ = 0.0f;
    motion_state_ = RotatorMotionState::Idle;
  } else {
    motion_state_ = (current_speed_deg_per_sec_ > 0.0f)
                        ? RotatorMotionState::Clockwise
                        : RotatorMotionState::CounterClockwise;
  }

  if (direction_sign != 0) {
    last_motion_direction_ = direction_sign;
  }
}

float RotatorAxisSimulator::current_angle() const { return current_angle_deg_; }

float RotatorAxisSimulator::target_angle() const { return target_angle_deg_; }

float RotatorAxisSimulator::current_speed_deg_per_sec() const {
  return current_speed_deg_per_sec_;
}

RotatorMotionState RotatorAxisSimulator::motion_state() const { return motion_state_; }

std::optional<float> RotatorAxisSimulator::read_sensor() {
  if (fault_sensor_dropout_) {
    return std::nullopt;
  }

  if (config_.sensor_noise_stddev_deg <= 0.0f) {
    return current_angle_deg_;
  }

  std::normal_distribution<float> dist(0.0f, config_.sensor_noise_stddev_deg);
  return normalize_angle(current_angle_deg_ + dist(rng_));
}

float RotatorAxisSimulator::normalize_angle(float angle_deg) const {
  if (!config_.wraparound) {
    return clamp_non_wrap_angle(angle_deg);
  }

  float normalized = std::fmod(angle_deg, 360.0f);
  if (normalized < 0.0f) {
    normalized += 360.0f;
  }
  return normalized;
}

float RotatorAxisSimulator::shortest_signed_delta(float from_deg, float to_deg) const {
  const float from_normalized = normalize_angle(from_deg);
  const float to_normalized = normalize_angle(to_deg);

  if (!config_.wraparound) {
    return to_normalized - from_normalized;
  }

  float delta = to_normalized - from_normalized;
  if (delta > 180.0f) {
    delta -= 360.0f;
  } else if (delta < -180.0f) {
    delta += 360.0f;
  }
  return delta;
}

float RotatorAxisSimulator::clamp_non_wrap_angle(float angle_deg) const {
  return clampf(angle_deg, config_.min_angle_deg, config_.max_angle_deg);
}

DualAxisRotatorSimulator::DualAxisRotatorSimulator(const DualAxisRotatorSimConfig &config)
    : azimuth_(config.azimuth), elevation_(config.elevation) {}

void DualAxisRotatorSimulator::step_ms(uint32_t delta_ms) {
  azimuth_.step_ms(delta_ms);
  elevation_.step_ms(delta_ms);
}

void DualAxisRotatorSimulator::set_target_azimuth(float angle_deg) {
  azimuth_.set_target_angle(angle_deg);
}

void DualAxisRotatorSimulator::set_target_elevation(float angle_deg) {
  elevation_.set_target_angle(angle_deg);
}

void DualAxisRotatorSimulator::set_current_azimuth(float angle_deg) {
  azimuth_.set_current_angle(angle_deg);
}

void DualAxisRotatorSimulator::set_current_elevation(float angle_deg) {
  elevation_.set_current_angle(angle_deg);
}

float DualAxisRotatorSimulator::current_azimuth() const { return azimuth_.current_angle(); }

float DualAxisRotatorSimulator::current_elevation() const { return elevation_.current_angle(); }

float DualAxisRotatorSimulator::target_azimuth() const { return azimuth_.target_angle(); }

float DualAxisRotatorSimulator::target_elevation() const { return elevation_.target_angle(); }

RotatorMotionState DualAxisRotatorSimulator::azimuth_motion_state() const {
  return azimuth_.motion_state();
}

RotatorMotionState DualAxisRotatorSimulator::elevation_motion_state() const {
  return elevation_.motion_state();
}

std::optional<float> DualAxisRotatorSimulator::read_azimuth_sensor() {
  return azimuth_.read_sensor();
}

std::optional<float> DualAxisRotatorSimulator::read_elevation_sensor() {
  return elevation_.read_sensor();
}

void DualAxisRotatorSimulator::set_fault_active(
    RotatorAxisId axis,
    RotatorFault fault,
    bool active) {
  if (axis == RotatorAxisId::Azimuth) {
    azimuth_.set_fault_active(fault, active);
  } else {
    elevation_.set_fault_active(fault, active);
  }
}

bool DualAxisRotatorSimulator::is_fault_active(RotatorAxisId axis, RotatorFault fault) const {
  if (axis == RotatorAxisId::Azimuth) {
    return azimuth_.is_fault_active(fault);
  }
  return elevation_.is_fault_active(fault);
}

RotatorAxisSimulator &DualAxisRotatorSimulator::azimuth_axis() { return azimuth_; }

RotatorAxisSimulator &DualAxisRotatorSimulator::elevation_axis() { return elevation_; }

const RotatorAxisSimulator &DualAxisRotatorSimulator::azimuth_axis() const { return azimuth_; }

const RotatorAxisSimulator &DualAxisRotatorSimulator::elevation_axis() const {
  return elevation_;
}

RotatorSimulator::RotatorSimulator(const RotatorSimConfig &config)
    : axis_(RotatorAxisConfig{
          true,
          0.0f,
          360.0f,
          config.max_speed_deg_per_sec,
          config.accel_deg_per_sec2,
          config.tolerance_deg,
          config.sensor_noise_stddev_deg,
          0.0f,
          0.0f,
      }) {}

void RotatorSimulator::set_target_azimuth(float azimuth_deg) { axis_.set_target_angle(azimuth_deg); }

void RotatorSimulator::set_current_azimuth(float azimuth_deg) {
  axis_.set_current_angle(azimuth_deg);
}

void RotatorSimulator::set_max_speed(float max_speed_deg_per_sec) {
  axis_.set_max_speed(max_speed_deg_per_sec);
}

void RotatorSimulator::set_acceleration(float accel_deg_per_sec2) {
  axis_.set_acceleration(accel_deg_per_sec2);
}

void RotatorSimulator::set_sensor_noise(float stddev_deg) { axis_.set_sensor_noise(stddev_deg); }

void RotatorSimulator::step_ms(uint32_t delta_ms) { axis_.step_ms(delta_ms); }

float RotatorSimulator::current_azimuth() const { return axis_.current_angle(); }

float RotatorSimulator::target_azimuth() const { return axis_.target_angle(); }

float RotatorSimulator::current_speed_deg_per_sec() const {
  return axis_.current_speed_deg_per_sec();
}

RotatorMotionState RotatorSimulator::motion_state() const { return axis_.motion_state(); }

float RotatorSimulator::read_azimuth_sensor() {
  const auto reading = axis_.read_sensor();
  return reading.value_or(current_azimuth());
}

void RotatorSimulator::set_fault_active(RotatorFault fault, bool active) {
  axis_.set_fault_active(fault, active);
}

bool RotatorSimulator::is_fault_active(RotatorFault fault) const {
  return axis_.is_fault_active(fault);
}
