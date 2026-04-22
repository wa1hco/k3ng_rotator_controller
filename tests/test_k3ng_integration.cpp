// Integration tests: fork real K3NG firmware, drive it via Yaesu protocol,
// assert closed-loop behavior. Each test gets a fresh subprocess.
//
// Expected runtime: ~60s total (dominated by rotation time at 30 deg/s).
// Run with: make integration   (from tests/)
// Override binary: K3NG_BINARY=path/to/program make integration

#include <cmath>

#include "k3ng_process.h"
#include "test_framework.h"

// ── Smoke tests ───────────────────────────────────────────────────────────────

TEST_CASE(k3ng_starts_and_responds_to_c2) {
  K3ngProcess k3ng;
  float az = k3ng.query_az(1.0f);
  REQUIRE_TRUE(az >= 0.0f);          // got a response
  REQUIRE_TRUE(az >= 170.0f && az <= 190.0f);  // starts near home (180°)
}

// ── Basic move tests ──────────────────────────────────────────────────────────

TEST_CASE(k3ng_move_to_270_arrives_within_tolerance) {
  K3ngProcess k3ng;
  float az = k3ng.move_and_wait(270.0f, 20.0f);
  REQUIRE_TRUE(std::fabs(az - 270.0f) <= K3ngProcess::kDefaultTolerance);
}

TEST_CASE(k3ng_move_to_ccw_limit_180) {
  // Start by moving CW first so there's somewhere to come back from.
  K3ngProcess k3ng;
  k3ng.move_and_wait(270.0f, 20.0f);
  float az = k3ng.move_and_wait(180.0f, 20.0f);
  REQUIRE_TRUE(std::fabs(az - 180.0f) <= K3ngProcess::kDefaultTolerance);
}

TEST_CASE(k3ng_move_to_360) {
  K3ngProcess k3ng;
  float az = k3ng.move_and_wait(360.0f, 25.0f);
  REQUIRE_TRUE(std::fabs(az - 360.0f) <= K3ngProcess::kDefaultTolerance);
}

// ── Overlap zone ──────────────────────────────────────────────────────────────
// Bearings 0-90° are in the overlap zone: K3NG routes them to raw 360-450°
// (second pass over the NE quadrant) to avoid reversing direction.

TEST_CASE(k3ng_move_to_overlap_zone_090) {
  K3ngProcess k3ng;
  // Start from CW side so the approach is from the high end.
  k3ng.move_and_wait(270.0f, 20.0f);
  float az = k3ng.move_and_wait(90.0f, 25.0f);
  REQUIRE_TRUE(std::fabs(az - 90.0f) <= K3ngProcess::kDefaultTolerance);
}

TEST_CASE(k3ng_move_to_overlap_zone_045) {
  K3ngProcess k3ng;
  k3ng.move_and_wait(270.0f, 20.0f);
  float az = k3ng.move_and_wait(45.0f, 30.0f);
  REQUIRE_TRUE(std::fabs(az - 45.0f) <= K3ngProcess::kDefaultTolerance);
}

// ── Stop command ──────────────────────────────────────────────────────────────

TEST_CASE(k3ng_stop_command_halts_mid_rotation) {
  K3ngProcess k3ng;
  // Kick off a long move, then stop it after 1 second.
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "M%03d\r\n", 360);
  k3ng.send(cmd);
  usleep(1000000);  // 1s — rotor should be ~30° into the move

  float az_before = k3ng.query_az(0.5f);
  k3ng.send("A\r\n");
  // At 30°/s max and 30°/s² accel, full deceleration takes 1 second.
  usleep(1500000);  // 1.5s — enough for any speed to reach zero

  float az_after1 = k3ng.query_az(0.5f);
  usleep(300000);
  float az_after2 = k3ng.query_az(0.5f);

  // Should have moved appreciably from home before stop.
  REQUIRE_TRUE(az_before > 185.0f);
  // Should be settled (not still moving) after stop.
  REQUIRE_TRUE(std::fabs(az_after2 - az_after1) < 1.0f);
  // Should not have reached the target.
  REQUIRE_TRUE(az_after2 < 355.0f);
}

// ── Sequential moves ──────────────────────────────────────────────────────────

TEST_CASE(k3ng_sequential_moves_270_then_180) {
  K3ngProcess k3ng;
  float az1 = k3ng.move_and_wait(270.0f, 20.0f);
  float az2 = k3ng.move_and_wait(180.0f, 20.0f);
  REQUIRE_TRUE(std::fabs(az1 - 270.0f) <= K3ngProcess::kDefaultTolerance);
  REQUIRE_TRUE(std::fabs(az2 - 180.0f) <= K3ngProcess::kDefaultTolerance);
}
