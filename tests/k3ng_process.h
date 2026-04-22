// k3ng_process.h
// Manages a K3NG native firmware subprocess for integration testing.
// Each test instantiates one; the destructor kills it cleanly.

#pragma once

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

class K3ngProcess {
 public:
  // K3NG's AZIMUTH_TOLERANCE default is 3.0°; add 0.5° test margin.
  static constexpr float kDefaultTolerance = 3.5f;

  explicit K3ngProcess(const char *binary = nullptr) {
    start(binary ? binary : default_binary());
  }
  ~K3ngProcess() { stop(); }
  K3ngProcess(const K3ngProcess &) = delete;
  K3ngProcess &operator=(const K3ngProcess &) = delete;

  void send(const char *cmd) {
    if (cmd_fd_ >= 0) ::write(cmd_fd_, cmd, strlen(cmd));
  }

  // Send Mxxx and wait until azimuth settles within tolerance. Throws on timeout.
  float move_and_wait(float target_deg, float timeout_sec = 20.0f,
                      float tolerance_deg = kDefaultTolerance) {
    char cmd[16];
    snprintf(cmd, sizeof(cmd), "M%03d\r\n", (int)std::roundf(target_deg));
    send(cmd);
    return wait_settled(target_deg, timeout_sec, tolerance_deg);
  }

  // Poll C2 until az is stable within tolerance of target. Throws on timeout.
  float wait_settled(float target_deg, float timeout_sec,
                     float tolerance_deg = kDefaultTolerance) {
    using Clock = std::chrono::steady_clock;
    auto deadline = Clock::now() + std::chrono::duration<float>(timeout_sec);
    float az = -1.0f;
    while (Clock::now() < deadline) {
      send("C2\r\n");
      az = read_az(0.15f);
      if (az >= 0.0f && std::fabs(az - target_deg) <= tolerance_deg) {
        usleep(150000);
        send("C2\r\n");
        float az2 = read_az(0.15f);
        if (az2 >= 0.0f && std::fabs(az2 - target_deg) <= tolerance_deg &&
            std::fabs(az2 - az) < 0.5f)
          return az2;
      }
    }
    char msg[128];
    snprintf(msg, sizeof(msg), "timeout waiting for az=%.1f (last=%.1f)",
             target_deg, az);
    throw std::runtime_error(msg);
  }

  // Single C2 query; returns azimuth or -1.0 on timeout.
  float query_az(float timeout_sec = 0.5f) {
    send("C2\r\n");
    return read_az(timeout_sec);
  }

  float last_az() const { return last_az_; }

 private:
  pid_t pid_     = -1;
  int   cmd_fd_  = -1;
  int   resp_fd_ = -1;
  float last_az_ = -1.0f;
  std::string buf_;

  static const char *default_binary() {
    const char *env = getenv("K3NG_BINARY");
    return env ? env : "../.pio/build/native_test/program";
  }

  void start(const char *path) {
    int to_k3ng[2], from_k3ng[2];
    if (pipe(to_k3ng) < 0 || pipe(from_k3ng) < 0)
      throw std::runtime_error("pipe() failed");
    pid_ = fork();
    if (pid_ < 0) throw std::runtime_error("fork() failed");
    if (pid_ == 0) {
      dup2(to_k3ng[0],   STDIN_FILENO);
      dup2(from_k3ng[1], STDOUT_FILENO);
      int nfd = open("/dev/null", O_WRONLY);
      if (nfd >= 0) dup2(nfd, STDERR_FILENO);
      close(to_k3ng[0]);  close(to_k3ng[1]);
      close(from_k3ng[0]); close(from_k3ng[1]);
      execl(path, path, nullptr);
      _exit(1);
    }
    close(to_k3ng[0]);
    close(from_k3ng[1]);
    cmd_fd_  = to_k3ng[1];
    resp_fd_ = from_k3ng[0];
    fcntl(resp_fd_, F_SETFL, O_NONBLOCK);
    usleep(200000);  // K3NG initialization
  }

  void stop() {
    if (cmd_fd_  >= 0) { close(cmd_fd_);  cmd_fd_  = -1; }
    if (resp_fd_ >= 0) { close(resp_fd_); resp_fd_ = -1; }
    if (pid_ > 0) { kill(pid_, SIGTERM); waitpid(pid_, nullptr, 0); pid_ = -1; }
  }

  // Read resp_fd until AZ=nnn line found or timeout.
  float read_az(float timeout_sec) {
    using Clock = std::chrono::steady_clock;
    auto deadline = Clock::now() + std::chrono::duration<float>(timeout_sec);
    while (Clock::now() < deadline) {
      char tmp[256];
      ssize_t n;
      while ((n = ::read(resp_fd_, tmp, sizeof(tmp) - 1)) > 0) {
        tmp[n] = '\0';
        buf_ += tmp;
      }
      size_t pos;
      while ((pos = buf_.find('\n')) != std::string::npos) {
        std::string line = buf_.substr(0, pos);
        buf_ = buf_.substr(pos + 1);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.size() >= 4 && line.substr(0, 3) == "AZ=") {
          try {
            last_az_ = std::stof(line.substr(3));
            return last_az_;
          } catch (...) {}
        }
      }
      usleep(20000);
    }
    return -1.0f;
  }
};
