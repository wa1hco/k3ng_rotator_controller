#include "rotator_feature_config.h"

std::vector<std::string> validate_feature_config(const FeatureConfig &config) {
  std::vector<std::string> errors;

  if (config.feature_yaesu_emulation && config.feature_easycom_emulation) {
    errors.push_back("FEATURE_YAESU_EMULATION and FEATURE_EASYCOM_EMULATION cannot both be enabled");
  }

  if (config.feature_yaesu_emulation && config.feature_dcu_1_emulation) {
    errors.push_back("FEATURE_YAESU_EMULATION and FEATURE_DCU_1_EMULATION cannot both be enabled");
  }

  if (config.feature_dcu_1_emulation && config.feature_easycom_emulation) {
    errors.push_back("FEATURE_DCU_1_EMULATION and FEATURE_EASYCOM_EMULATION cannot both be enabled");
  }

  if (config.feature_dcu_1_emulation && config.feature_elevation_control) {
    errors.push_back("FEATURE_DCU_1_EMULATION does not support FEATURE_ELEVATION_CONTROL");
  }

  if (config.feature_master_with_serial_slave && config.feature_master_with_ethernet_slave) {
    errors.push_back("FEATURE_MASTER_WITH_SERIAL_SLAVE and FEATURE_MASTER_WITH_ETHERNET_SLAVE cannot both be enabled");
  }

  if (config.feature_remote_unit_slave &&
      (config.feature_master_with_serial_slave || config.feature_master_with_ethernet_slave)) {
    errors.push_back("A unit cannot be both FEATURE_REMOTE_UNIT_SLAVE and master");
  }

  if ((config.feature_az_position_get_from_remote_unit ||
       config.feature_el_position_get_from_remote_unit) &&
      !(config.feature_master_with_serial_slave || config.feature_master_with_ethernet_slave)) {
    errors.push_back("Remote position features require FEATURE_MASTER_WITH_SERIAL_SLAVE or FEATURE_MASTER_WITH_ETHERNET_SLAVE");
  }

  return errors;
}
