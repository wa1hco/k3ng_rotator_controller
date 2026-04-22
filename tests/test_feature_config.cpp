#include <algorithm>
#include <string>
#include <vector>

#include "test_framework.h"
#include "rotator_feature_config.h"

namespace {

bool contains_message(const std::vector<std::string> &messages, const std::string &needle) {
  return std::find(messages.begin(), messages.end(), needle) != messages.end();
}

}  // namespace

TEST_CASE(valid_configuration_has_no_errors) {
  FeatureConfig config;
  config.feature_yaesu_emulation = true;

  const auto errors = validate_feature_config(config);

  REQUIRE_TRUE(errors.empty());
}

TEST_CASE(yaesu_and_easycom_conflict_is_reported) {
  FeatureConfig config;
  config.feature_yaesu_emulation = true;
  config.feature_easycom_emulation = true;

  const auto errors = validate_feature_config(config);

  REQUIRE_TRUE(contains_message(
      errors,
      "FEATURE_YAESU_EMULATION and FEATURE_EASYCOM_EMULATION cannot both be enabled"));
}

TEST_CASE(remote_position_requires_master_link) {
  FeatureConfig config;
  config.feature_az_position_get_from_remote_unit = true;

  const auto errors = validate_feature_config(config);

  REQUIRE_TRUE(contains_message(
      errors,
      "Remote position features require FEATURE_MASTER_WITH_SERIAL_SLAVE or FEATURE_MASTER_WITH_ETHERNET_SLAVE"));
}

TEST_CASE(slave_cannot_also_be_master) {
  FeatureConfig config;
  config.feature_remote_unit_slave = true;
  config.feature_master_with_serial_slave = true;

  const auto errors = validate_feature_config(config);

  REQUIRE_TRUE(contains_message(
      errors,
      "A unit cannot be both FEATURE_REMOTE_UNIT_SLAVE and master"));
}
