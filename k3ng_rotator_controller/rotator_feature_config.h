#ifndef ROTATOR_FEATURE_CONFIG_H
#define ROTATOR_FEATURE_CONFIG_H

#include <string>
#include <vector>

struct FeatureConfig {
  bool feature_yaesu_emulation = false;
  bool feature_easycom_emulation = false;
  bool feature_dcu_1_emulation = false;
  bool feature_elevation_control = false;
  bool feature_remote_unit_slave = false;
  bool feature_master_with_serial_slave = false;
  bool feature_master_with_ethernet_slave = false;
  bool feature_az_position_get_from_remote_unit = false;
  bool feature_el_position_get_from_remote_unit = false;
};

std::vector<std::string> validate_feature_config(const FeatureConfig &config);

#endif  // ROTATOR_FEATURE_CONFIG_H
