#include "rotator_protocol_shim.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace {

std::string format_optional_deg(const std::optional<float> &value) {
  if (!value.has_value()) {
    return "---.-";
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << value.value();
  return oss.str();
}

std::string format_deg(float value) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << value;
  return oss.str();
}

}  // namespace

RotatorProtocolShim::RotatorProtocolShim(DualAxisRotatorSimulator *simulator)
    : simulator_(simulator) {
  poll_display();
}

std::string RotatorProtocolShim::handle_command(const std::string &command_line) {
  const std::string trimmed = trim(command_line);
  if (trimmed.empty()) {
    return "ERR";
  }

  const std::string upper = to_upper(trimmed);

  if (upper == "C") {
    poll_display();
    return "AZ " + format_optional_deg(display_state_.azimuth_readout_deg);
  }

  if (upper == "C2") {
    poll_display();
    return "AZ " + format_optional_deg(display_state_.azimuth_readout_deg) +
           " EL " + format_optional_deg(display_state_.elevation_readout_deg);
  }

  if (upper == "S") {
    simulator_->set_target_azimuth(simulator_->current_azimuth());
    simulator_->set_target_elevation(simulator_->current_elevation());
    poll_display();
    return "OK";
  }

  if (upper == "SA") {
    simulator_->set_target_azimuth(simulator_->current_azimuth());
    poll_display();
    return "OK";
  }

  if (upper == "SE") {
    simulator_->set_target_elevation(simulator_->current_elevation());
    poll_display();
    return "OK";
  }

  if (upper == "?DS") {
    poll_display();
    return "CTRL AZ=" + format_optional_deg(display_state_.azimuth_readout_deg) +
           " EL=" + format_optional_deg(display_state_.elevation_readout_deg) +
           " TAZ=" + format_deg(display_state_.azimuth_target_deg) +
           " TEL=" + format_deg(display_state_.elevation_target_deg);
  }

  if (!upper.empty() && upper[0] == 'M') {
    const std::string payload = trim(trimmed.substr(1));
    if (payload.empty()) {
      return "ERR";
    }

    try {
      const float az = std::stof(payload);
      simulator_->set_target_azimuth(az);
      poll_display();
      return "OK";
    } catch (...) {
      return "ERR";
    }
  }

  if (!upper.empty() && upper[0] == 'W') {
    std::istringstream iss(trimmed.substr(1));
    float az = 0.0f;
    float el = 0.0f;
    if (!(iss >> az >> el)) {
      return "ERR";
    }

    simulator_->set_target_azimuth(az);
    simulator_->set_target_elevation(el);
    poll_display();
    return "OK";
  }

  return "ERR";
}

void RotatorProtocolShim::poll_display() {
  display_state_.azimuth_readout_deg = simulator_->read_azimuth_sensor();
  display_state_.elevation_readout_deg = simulator_->read_elevation_sensor();
  display_state_.azimuth_target_deg = simulator_->target_azimuth();
  display_state_.elevation_target_deg = simulator_->target_elevation();
  display_state_.azimuth_active =
      simulator_->azimuth_motion_state() != RotatorMotionState::Idle;
  display_state_.elevation_active =
      simulator_->elevation_motion_state() != RotatorMotionState::Idle;
}

ControllerDisplayState RotatorProtocolShim::display_state() const {
  return display_state_;
}

std::string RotatorProtocolShim::trim(const std::string &text) {
  size_t start = 0;
  while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])) != 0) {
    ++start;
  }

  size_t end = text.size();
  while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
    --end;
  }

  return text.substr(start, end - start);
}

std::string RotatorProtocolShim::to_upper(const std::string &text) {
  std::string upper = text;
  std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return upper;
}
