#ifndef ROTATOR_PROTOCOL_SHIM_H
#define ROTATOR_PROTOCOL_SHIM_H

#include <optional>
#include <string>

#include "rotator_simulator.h"

struct ControllerDisplayState {
  std::optional<float> azimuth_readout_deg;
  std::optional<float> elevation_readout_deg;
  float azimuth_target_deg = 0.0f;
  float elevation_target_deg = 0.0f;
  bool azimuth_active = false;
  bool elevation_active = false;
};

class RotatorProtocolShim {
 public:
  explicit RotatorProtocolShim(DualAxisRotatorSimulator *simulator);

  // Handles a small practical subset of Yaesu/Easycom style commands.
  std::string handle_command(const std::string &command_line);

  void poll_display();
  ControllerDisplayState display_state() const;

 private:
  static std::string trim(const std::string &text);
  static std::string to_upper(const std::string &text);

  DualAxisRotatorSimulator *simulator_;
  ControllerDisplayState display_state_;
};

#endif  // ROTATOR_PROTOCOL_SHIM_H
