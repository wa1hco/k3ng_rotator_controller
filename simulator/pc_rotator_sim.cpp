#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include "rotator_protocol_shim.h"
#include "rotator_simulator.h"

namespace {

void print_help() {
  std::cout << "Commands:\n"
            << "  status                     Show both controller and hardware displays\n"
            << "  set <deg>                  Set target azimuth\n"
            << "  setel <deg>                Set target elevation\n"
            << "  az <deg>                   Force current azimuth\n"
            << "  el <deg>                   Force current elevation\n"
            << "  step <ms>                  Advance simulation by milliseconds\n"
            << "  run <ms> [step_ms]         Advance repeatedly (default step=20ms)\n"
            << "  speed <az|el|both> <deg_s> Set max speed\n"
            << "  accel <az|el|both> <deg_s2> Set acceleration\n"
            << "  noise <az|el|both> <stddev> Set sensor noise\n"
            << "  backlash <az|el> <deg>     Set backlash\n"
            << "  stiction <az|el> <deg_s>   Set stiction threshold\n"
            << "  fault <az|el> <name> <on|off>  fault name: stall|dropout|stuck_cw|stuck_ccw\n"
            << "  sensor                     Print controller sensor readout\n"
            << "  display                    Print dual display panel\n"
            << "  proto <cmd>                Send protocol command (C,C2,M,W,S,SA,SE,?DS)\n"
            << "  help                       Show this help\n"
            << "  quit                       Exit\n";
}

const char *motion_state_to_string(RotatorMotionState state) {
  switch (state) {
    case RotatorMotionState::Idle:
      return "IDLE";
    case RotatorMotionState::Clockwise:
      return "CW";
    case RotatorMotionState::CounterClockwise:
      return "CCW";
  }
  return "UNKNOWN";
}

std::string value_or_na(const std::optional<float> &value) {
  if (!value.has_value()) {
    return "---.-";
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << value.value();
  return oss.str();
}

void print_display(const DualAxisRotatorSimulator &sim, const RotatorProtocolShim &protocol) {
  const ControllerDisplayState ctrl = protocol.display_state();

  std::cout << "\n=== Controller Software Display ===\n"
            << "AZ(read)=" << value_or_na(ctrl.azimuth_readout_deg)
            << "  EL(read)=" << value_or_na(ctrl.elevation_readout_deg)
            << "  TAZ=" << std::fixed << std::setprecision(2) << ctrl.azimuth_target_deg
            << "  TEL=" << ctrl.elevation_target_deg
            << "  AZ_ACTIVE=" << (ctrl.azimuth_active ? "Y" : "N")
            << "  EL_ACTIVE=" << (ctrl.elevation_active ? "Y" : "N") << "\n";

  std::cout << "=== Simulated Hardware Position ===\n";
  std::cout << std::fixed << std::setprecision(2)
            << "AZ(actual)=" << sim.current_azimuth() << " TARGET=" << sim.target_azimuth()
            << " STATE=" << motion_state_to_string(sim.azimuth_motion_state()) << "\n"
            << "EL(actual)=" << sim.current_elevation() << " TARGET=" << sim.target_elevation()
            << " STATE=" << motion_state_to_string(sim.elevation_motion_state()) << "\n\n";
}

bool parse_axis(const std::string &token, RotatorAxisId &axis) {
  if (token == "az") {
    axis = RotatorAxisId::Azimuth;
    return true;
  }
  if (token == "el") {
    axis = RotatorAxisId::Elevation;
    return true;
  }
  return false;
}

bool parse_fault(const std::string &token, RotatorFault &fault) {
  if (token == "stall") {
    fault = RotatorFault::Stall;
    return true;
  }
  if (token == "dropout") {
    fault = RotatorFault::SensorDropout;
    return true;
  }
  if (token == "stuck_cw") {
    fault = RotatorFault::StuckClockwiseRelay;
    return true;
  }
  if (token == "stuck_ccw") {
    fault = RotatorFault::StuckCounterClockwiseRelay;
    return true;
  }
  return false;
}

}  // namespace

int main() {
  DualAxisRotatorSimConfig config;
  config.azimuth.wraparound = true;
  config.azimuth.min_angle_deg = 0.0f;
  config.azimuth.max_angle_deg = 360.0f;
  config.elevation.wraparound = false;
  config.elevation.min_angle_deg = 0.0f;
  config.elevation.max_angle_deg = 180.0f;

  DualAxisRotatorSimulator sim(config);
  RotatorProtocolShim protocol(&sim);

  std::cout << "K3NG PC Rotator Simulator (dual-axis)\n";
  print_help();
  protocol.poll_display();
  print_display(sim, protocol);

  std::string line;
  while (std::cout << "> " && std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty()) {
      continue;
    }

    if (cmd == "quit" || cmd == "exit") {
      break;
    }

    if (cmd == "help") {
      print_help();
      continue;
    }

    if (cmd == "status" || cmd == "display") {
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "sensor") {
      protocol.poll_display();
      const auto display = protocol.display_state();
      std::cout << "AZ=" << value_or_na(display.azimuth_readout_deg)
                << " EL=" << value_or_na(display.elevation_readout_deg) << "\n";
      continue;
    }

    if (cmd == "set") {
      float value = 0.0f;
      if (!(iss >> value)) {
        std::cerr << "Expected: set <deg>\n";
        continue;
      }
      sim.set_target_azimuth(value);
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "setel") {
      float value = 0.0f;
      if (!(iss >> value)) {
        std::cerr << "Expected: setel <deg>\n";
        continue;
      }
      sim.set_target_elevation(value);
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "az") {
      float value = 0.0f;
      if (!(iss >> value)) {
        std::cerr << "Expected: az <deg>\n";
        continue;
      }
      sim.set_current_azimuth(value);
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "el") {
      float value = 0.0f;
      if (!(iss >> value)) {
        std::cerr << "Expected: el <deg>\n";
        continue;
      }
      sim.set_current_elevation(value);
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "step") {
      int ms = 0;
      if (!(iss >> ms) || ms < 0) {
        std::cerr << "Expected: step <ms> where ms >= 0\n";
        continue;
      }
      sim.step_ms(static_cast<uint32_t>(ms));
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "run") {
      int total_ms = 0;
      int step_ms = 20;
      if (!(iss >> total_ms) || total_ms < 0) {
        std::cerr << "Expected: run <ms> [step_ms]\n";
        continue;
      }
      if ((iss >> step_ms) && step_ms <= 0) {
        std::cerr << "step_ms must be > 0\n";
        continue;
      }
      if (step_ms <= 0) {
        step_ms = 20;
      }

      int elapsed = 0;
      while (elapsed < total_ms) {
        const int slice = std::min(step_ms, total_ms - elapsed);
        sim.step_ms(static_cast<uint32_t>(slice));
        elapsed += slice;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "speed") {
      std::string axis;
      float value = 0.0f;
      if (!(iss >> axis >> value) || value < 0.0f) {
        std::cerr << "Expected: speed <az|el|both> <deg_per_sec>\n";
        continue;
      }
      if (axis == "az" || axis == "both") {
        sim.azimuth_axis().set_max_speed(value);
      }
      if (axis == "el" || axis == "both") {
        sim.elevation_axis().set_max_speed(value);
      }
      if (axis != "az" && axis != "el" && axis != "both") {
        std::cerr << "Axis must be az, el, or both\n";
        continue;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "accel") {
      std::string axis;
      float value = 0.0f;
      if (!(iss >> axis >> value) || value < 0.0f) {
        std::cerr << "Expected: accel <az|el|both> <deg_per_sec2>\n";
        continue;
      }
      if (axis == "az" || axis == "both") {
        sim.azimuth_axis().set_acceleration(value);
      }
      if (axis == "el" || axis == "both") {
        sim.elevation_axis().set_acceleration(value);
      }
      if (axis != "az" && axis != "el" && axis != "both") {
        std::cerr << "Axis must be az, el, or both\n";
        continue;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "noise") {
      std::string axis;
      float value = 0.0f;
      if (!(iss >> axis >> value) || value < 0.0f) {
        std::cerr << "Expected: noise <az|el|both> <stddev_deg>\n";
        continue;
      }
      if (axis == "az" || axis == "both") {
        sim.azimuth_axis().set_sensor_noise(value);
      }
      if (axis == "el" || axis == "both") {
        sim.elevation_axis().set_sensor_noise(value);
      }
      if (axis != "az" && axis != "el" && axis != "both") {
        std::cerr << "Axis must be az, el, or both\n";
        continue;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "backlash") {
      std::string axis;
      float value = 0.0f;
      if (!(iss >> axis >> value) || value < 0.0f) {
        std::cerr << "Expected: backlash <az|el> <deg>\n";
        continue;
      }
      if (axis == "az") {
        sim.azimuth_axis().set_backlash(value);
      } else if (axis == "el") {
        sim.elevation_axis().set_backlash(value);
      } else {
        std::cerr << "Axis must be az or el\n";
        continue;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "stiction") {
      std::string axis;
      float value = 0.0f;
      if (!(iss >> axis >> value) || value < 0.0f) {
        std::cerr << "Expected: stiction <az|el> <deg_per_sec>\n";
        continue;
      }
      if (axis == "az") {
        sim.azimuth_axis().set_stiction(value);
      } else if (axis == "el") {
        sim.elevation_axis().set_stiction(value);
      } else {
        std::cerr << "Axis must be az or el\n";
        continue;
      }
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "fault") {
      std::string axis_name;
      std::string fault_name;
      std::string state;
      if (!(iss >> axis_name >> fault_name >> state)) {
        std::cerr << "Expected: fault <az|el> <stall|dropout|stuck_cw|stuck_ccw> <on|off>\n";
        continue;
      }

      RotatorAxisId axis;
      RotatorFault fault;
      if (!parse_axis(axis_name, axis) || !parse_fault(fault_name, fault)) {
        std::cerr << "Invalid axis or fault\n";
        continue;
      }

      if (state != "on" && state != "off") {
        std::cerr << "Fault state must be on or off\n";
        continue;
      }

      sim.set_fault_active(axis, fault, state == "on");
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    if (cmd == "proto") {
      std::string protocol_command;
      std::getline(iss, protocol_command);
      const std::string response = protocol.handle_command(protocol_command);
      std::cout << response << "\n";
      protocol.poll_display();
      print_display(sim, protocol);
      continue;
    }

    std::cerr << "Unknown command: " << cmd << "\n";
  }

  return EXIT_SUCCESS;
}
