// gui_server.cpp
// HTTP bridge between the browser GUI and a K3NG native firmware subprocess.
// Spawns the K3NG binary, communicates via pipes using Yaesu GS-232B protocol.

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

constexpr int kDefaultPort   = 8080;
constexpr int kListenBacklog = 8;
constexpr int kReadBufSize   = 8192;
// How often to query K3NG position (ms)
constexpr int kPollIntervalMs = 100;

// ── HTTP helpers ─────────────────────────────────────────────────────────────

std::string url_decode(const std::string &input) {
  std::string output;
  output.reserve(input.size());
  for (size_t i = 0; i < input.size(); ++i) {
    if (input[i] == '%' && i + 2 < input.size()) {
      const std::string hex = input.substr(i + 1, 2);
      char *end = nullptr;
      const long value = std::strtol(hex.c_str(), &end, 16);
      if (end && *end == '\0') { output.push_back((char)value); i += 2; continue; }
    }
    output.push_back(input[i] == '+' ? ' ' : input[i]);
  }
  return output;
}

std::string get_query_value(const std::string &query, const std::string &key) {
  std::istringstream iss(query);
  std::string token;
  while (std::getline(iss, token, '&')) {
    const auto pos = token.find('=');
    const std::string raw_key = (pos == std::string::npos) ? token : token.substr(0, pos);
    if (raw_key == key)
      return (pos == std::string::npos) ? "" : url_decode(token.substr(pos + 1));
  }
  return "";
}

std::string make_http_response(int code, const char *text,
                               const char *ct, const std::string &body) {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << code << " " << text << "\r\n"
      << "Content-Type: " << ct << "\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Cache-Control: no-store\r\n"
      << "Connection: close\r\n\r\n"
      << body;
  return oss.str();
}

bool send_all(int fd, const std::string &data) {
  size_t sent = 0;
  while (sent < data.size()) {
    ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
    if (n <= 0) return false;
    sent += (size_t)n;
  }
  return true;
}

std::string read_text_file(const std::string &path) {
  std::ifstream f(path);
  if (!f) return "";
  std::ostringstream oss; oss << f.rdbuf(); return oss.str();
}

const char *mime_for(const std::string &p) {
  if (p.size() >= 5 && p.substr(p.size()-5) == ".html") return "text/html; charset=utf-8";
  if (p.size() >= 3 && p.substr(p.size()-3) == ".js")   return "application/javascript; charset=utf-8";
  if (p.size() >= 4 && p.substr(p.size()-4) == ".css")  return "text/css; charset=utf-8";
  return "text/plain; charset=utf-8";
}

// ── K3NG subprocess bridge ────────────────────────────────────────────────────

struct K3ngBridge {
  pid_t pid     = -1;
  int   cmd_fd  = -1;   // write Yaesu commands → K3NG stdin
  int   resp_fd = -1;   // read K3NG stdout responses

  // Parsed position state
  float az_deg      = 180.0f;  // compass bearing K3NG reports
  float speed_dps   = 0.0f;    // inferred from consecutive readings
  std::string direction = "IDLE";

  // Command-side state (tracked here, not queried from K3NG)
  std::string mode       = "IDLE";
  float       target_deg = 180.0f;

  // Timing
  using Clock = std::chrono::steady_clock;
  Clock::time_point last_poll     = Clock::now();
  Clock::time_point last_pos_time = Clock::now();

  // Partial response line buffer
  std::string resp_buf;

  // ── Lifecycle ─────────────────────────────────────────────────────────────

  bool start(const char *binary_path) {
    int to_k3ng[2], from_k3ng[2];
    if (pipe(to_k3ng) < 0 || pipe(from_k3ng) < 0) return false;

    pid = fork();
    if (pid < 0) return false;

    if (pid == 0) {
      // child: wire pipes and exec K3NG
      dup2(to_k3ng[0],   STDIN_FILENO);
      dup2(from_k3ng[1], STDOUT_FILENO);
      int null_fd = open("/dev/null", O_WRONLY);
      if (null_fd >= 0) dup2(null_fd, STDERR_FILENO);
      close(to_k3ng[0]);  close(to_k3ng[1]);
      close(from_k3ng[0]); close(from_k3ng[1]);
      execl(binary_path, binary_path, nullptr);
      _exit(1);
    }

    // parent: keep write end of stdin pipe and read end of stdout pipe
    close(to_k3ng[0]);
    close(from_k3ng[1]);
    cmd_fd  = to_k3ng[1];
    resp_fd = from_k3ng[0];
    fcntl(resp_fd, F_SETFL, O_NONBLOCK);

    last_poll = last_pos_time = Clock::now();
    return true;
  }

  // ── I/O ──────────────────────────────────────────────────────────────────

  void send_yaesu(const char *cmd) {
    if (cmd_fd >= 0) write(cmd_fd, cmd, strlen(cmd));
  }

  // Drain all pending K3NG stdout bytes and parse any complete lines.
  void drain_responses() {
    char buf[256];
    ssize_t n;
    while ((n = ::read(resp_fd, buf, sizeof(buf) - 1)) > 0) {
      buf[n] = '\0';
      resp_buf += buf;
    }
    size_t pos;
    while ((pos = resp_buf.find('\n')) != std::string::npos) {
      std::string line = resp_buf.substr(0, pos);
      resp_buf = resp_buf.substr(pos + 1);
      if (!line.empty() && line.back() == '\r') line.pop_back();
      parse_line(line);
    }
  }

  // Parse "AZ=270EL=000" (K3NG C2 response).
  void parse_line(const std::string &line) {
    if (line.size() < 6 || line.substr(0, 3) != "AZ=") return;
    try {
      float new_az = std::stof(line.substr(3));
      auto  now    = Clock::now();
      float dt     = std::chrono::duration<float>(now - last_pos_time).count();
      if (dt > 0.01f) {
        float delta = new_az - az_deg;
        speed_dps = std::fabs(delta) / dt;
        if      (speed_dps < 0.5f) direction = "IDLE";
        else if (delta > 0)        direction = "CW";
        else                       direction = "CCW";
      }
      az_deg      = new_az;
      last_pos_time = now;

      // Auto-clear AUTO_TARGET mode once we arrive
      if (mode == "AUTO_TARGET" &&
          std::fabs(az_deg - target_deg) < 1.5f &&
          direction == "IDLE") {
        mode = "IDLE";
      }
    } catch (...) {}
  }

  // ── Periodic poll ─────────────────────────────────────────────────────────

  void tick() {
    drain_responses();
    auto now = Clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now - last_poll).count();
    if (elapsed >= kPollIntervalMs) {
      send_yaesu("C2\r\n");
      last_poll = now;
    }
  }

  // ── JSON state ────────────────────────────────────────────────────────────
  // K3NG C2 reports compass bearing 0-360.
  // GUI's hardwareAz field drives the compass needle; it uses az % 360 for
  // direction and az > 360 for the overlap-zone indicator.  For now we report
  // the bearing directly; the overlap indicator won't fire, but position is
  // correct.  (A future improvement: reconstruct raw angle from bearing + mode.)

  std::string state_json() const {
    std::ostringstream oss;
    oss << "{"
        << "\"controllerAz\":"    << az_deg      << ","
        << "\"controllerTarget\":" << target_deg  << ","
        << "\"mode\":\""           << mode        << "\","
        << "\"hardwareAz\":"       << az_deg      << ","
        << "\"hardwareSpeed\":"    << speed_dps   << ","
        << "\"hardwareDirection\":\"" << direction << "\","
        << "\"secondsPer360\":"    << 15.0f        // 450° range / 30°/s
        << "}";
    return oss.str();
  }
};

// ── API command handler ───────────────────────────────────────────────────────

std::string handle_command(K3ngBridge &b, const std::string &query) {
  const std::string action = get_query_value(query, "action");
  const std::string value  = get_query_value(query, "value");

  if (action == "cw") {
    // Rotate to maximum CW bearing (90° = 450° raw via overlap logic)
    b.send_yaesu("M090\r\n");
    b.target_deg = 90.0f;
    b.mode = "MANUAL_CW";
  } else if (action == "ccw") {
    // Rotate to minimum CCW bearing (180° = 180° raw, the start of the range)
    b.send_yaesu("M180\r\n");
    b.target_deg = 180.0f;
    b.mode = "MANUAL_CCW";
  } else if (action == "stop") {
    b.send_yaesu("A\r\n");
    b.mode = "IDLE";
  } else if (action == "target") {
    try {
      float tgt = std::stof(value);
      tgt = std::max(0.0f, std::min(360.0f, tgt));
      char cmd[16];
      snprintf(cmd, sizeof(cmd), "M%03d\r\n", (int)std::roundf(tgt));
      b.send_yaesu(cmd);
      b.target_deg = tgt;
      b.mode = "AUTO_TARGET";
    } catch (...) {
      return "{\"ok\":false,\"error\":\"bad target\"}";
    }
  } else if (action == "speed") {
    // Speed slider not connected to K3NG firmware; ignore silently.
  } else {
    return "{\"ok\":false,\"error\":\"unknown action\"}";
  }
  return "{\"ok\":true}";
}

// ── Static file path resolution ───────────────────────────────────────────────

std::string resolve_file(const std::string &path) {
  if (path == "/" || path == "/gui" || path == "/gui/") return "gui/index.html";
  if (path == "/styles.css"  || path == "/gui/styles.css") return "gui/styles.css";
  if (path == "/app.js"      || path == "/gui/app.js")     return "gui/app.js";
  return "";
}

}  // namespace

// ── main ─────────────────────────────────────────────────────────────────────

int main(int argc, char **argv) {
  int         port          = kDefaultPort;
  const char *k3ng_binary   = "../.pio/build/native_test/program";

  for (int i = 1; i < argc; i++) {
    if (i + 1 < argc && std::string(argv[i]) == "--port")   { port = std::atoi(argv[++i]); }
    else if (i + 1 < argc && std::string(argv[i]) == "--k3ng") { k3ng_binary = argv[++i]; }
    else { k3ng_binary = argv[i]; }
  }

  signal(SIGPIPE, SIG_IGN);   // don't crash on K3NG subprocess death
  signal(SIGCHLD, SIG_IGN);   // auto-reap child

  K3ngBridge bridge;
  if (!bridge.start(k3ng_binary)) {
    std::cerr << "Failed to start: " << k3ng_binary << "\n";
    return 1;
  }
  std::cerr << "K3NG pid=" << bridge.pid << "  binary=" << k3ng_binary << "\n";

  // ── HTTP socket setup ─────────────────────────────────────────────────────
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) { std::perror("socket"); return 1; }
  int yes = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  sockaddr_in addr{};
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port        = htons((uint16_t)port);
  if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0) { std::perror("bind"); return 1; }
  listen(server_fd, kListenBacklog);
  std::cout << "GUI server: http://localhost:" << port << "/\n";

  // ── Event loop ────────────────────────────────────────────────────────────
  while (true) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(server_fd,     &rfds);
    FD_SET(bridge.resp_fd, &rfds);
    int maxfd = std::max(server_fd, bridge.resp_fd) + 1;
    struct timeval tv = {0, 50000};   // 50 ms wake-up
    select(maxfd, &rfds, nullptr, nullptr, &tv);

    bridge.tick();    // drain responses + maybe send C2

    if (!FD_ISSET(server_fd, &rfds)) continue;

    // ── Handle one HTTP request ───────────────────────────────────────────
    sockaddr_in caddr{};
    socklen_t   clen = sizeof(caddr);
    int client_fd = accept(server_fd, (sockaddr *)&caddr, &clen);
    if (client_fd < 0) continue;

    char buf[kReadBufSize];
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) { close(client_fd); continue; }
    buf[n] = '\0';

    std::istringstream req{std::string(buf)};
    std::string method, target, version;
    req >> method >> target >> version;

    std::string path  = target;
    std::string query;
    auto qpos = target.find('?');
    if (qpos != std::string::npos) { path = target.substr(0, qpos); query = target.substr(qpos + 1); }

    std::string response;
    if (method != "GET") {
      response = make_http_response(405, "Method Not Allowed", "text/plain", "Only GET\n");
    } else if (path == "/api/state") {
      response = make_http_response(200, "OK", "application/json", bridge.state_json());
    } else if (path == "/api/command") {
      response = make_http_response(200, "OK", "application/json", handle_command(bridge, query));
    } else {
      std::string fp = resolve_file(path);
      if (!fp.empty()) {
        std::string body = read_text_file(fp);
        if (!body.empty()) response = make_http_response(200, "OK", mime_for(fp), body);
        else               response = make_http_response(404, "Not Found", "text/plain", "Not found\n");
      } else {
        response = make_http_response(404, "Not Found", "text/plain", "Not found\n");
      }
    }

    send_all(client_fd, response);
    close(client_fd);
  }

  close(server_fd);
  return 0;
}
