const state = {
  hardwareAzDeg: 0,
  controllerReadoutAzDeg: 0,
  targetAzDeg: 0,
  currentSpeedDegPerSec: 0,
  mode: "IDLE",
  hardwareDirection: "IDLE",
  connected: false,
};

const ui = {
  ctrlAz: document.getElementById("ctrl-az"),
  ctrlTarget: document.getElementById("ctrl-target"),
  ctrlMode: document.getElementById("ctrl-mode"),
  hwAz: document.getElementById("hw-az"),
  hwSpeed: document.getElementById("hw-speed"),
  hwDir: document.getElementById("hw-dir"),
  btnCw: document.getElementById("btn-cw"),
  btnCcw: document.getElementById("btn-ccw"),
  btnStop: document.getElementById("btn-stop"),
  btnGoTarget: document.getElementById("btn-go-target"),
  targetInput: document.getElementById("target-input"),
  secondsSlider: document.getElementById("seconds-per-rotation"),
  secondsValue: document.getElementById("seconds-value"),
  dial: document.getElementById("dial"),
};

const ctx = ui.dial.getContext("2d");

function formatDeg(value) {
  return `${Number(value).toFixed(1)} deg`;
}

async function sendCommand(action, value = undefined) {
  const params = new URLSearchParams({ action });
  if (value !== undefined) {
    params.set("value", String(value));
  }

  try {
    const response = await fetch(`/api/command?${params.toString()}`);
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    state.connected = true;
  } catch (error) {
    state.connected = false;
  }
}

ui.btnCw.addEventListener("click", () => {
  sendCommand("cw");
});

ui.btnCcw.addEventListener("click", () => {
  sendCommand("ccw");
});

ui.btnStop.addEventListener("click", () => {
  sendCommand("stop");
});

ui.btnGoTarget.addEventListener("click", () => {
  const input = Number(ui.targetInput.value);
  if (!Number.isFinite(input)) {
    return;
  }
  sendCommand("target", input);
});

ui.secondsSlider.addEventListener("input", () => {
  const seconds = Number(ui.secondsSlider.value);
  ui.secondsValue.textContent = `${seconds} s`;
  sendCommand("speed", seconds);
});

async function pollState() {
  try {
    const response = await fetch("/api/state");
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const payload = await response.json();

    state.controllerReadoutAzDeg = payload.controllerAz ?? state.hardwareAzDeg;
    state.targetAzDeg = payload.controllerTarget ?? state.targetAzDeg;
    state.mode = payload.mode ?? "IDLE";
    state.hardwareAzDeg = payload.hardwareAz ?? state.hardwareAzDeg;
    state.currentSpeedDegPerSec = payload.hardwareSpeed ?? 0;
    state.hardwareDirection = payload.hardwareDirection ?? "IDLE";
    state.connected = true;

    if (typeof payload.secondsPer360 === "number") {
      ui.secondsSlider.value = String(payload.secondsPer360);
      ui.secondsValue.textContent = `${payload.secondsPer360.toFixed(0)} s`;
    }
  } catch (error) {
    state.connected = false;
  }
}

// Compass angle (0=N, CW) to canvas angle (0=E, CW)
function compassToCanvas(deg) {
  return (deg - 90) * (Math.PI / 180);
}

const kMaxAzDeg = 450;

function drawDial() {
  const w = ui.dial.width;
  const h = ui.dial.height;
  const cx = w / 2;
  const cy = h / 2;
  const r = Math.min(w, h) * 0.43;

  ctx.clearRect(0, 0, w, h);

  // --- Overlap zone: compass 0-90 deg (= mechanical 360-450) shaded orange ---
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.arc(cx, cy, r, compassToCanvas(0), compassToCanvas(90), false);
  ctx.closePath();
  ctx.fillStyle = "rgba(255, 140, 0, 0.18)";
  ctx.fill();

  // --- Compass face circle ---
  ctx.beginPath();
  ctx.arc(cx, cy, r, 0, Math.PI * 2);
  ctx.fillStyle = "rgba(0,0,0,0)";  // transparent fill (keep shading)
  ctx.fill();
  ctx.strokeStyle = "#2a3d5d";
  ctx.lineWidth = 2;
  ctx.stroke();

  // --- Tick marks and compass labels ---
  const compassLabels = { 0: "N", 90: "E", 180: "S", 270: "W" };
  for (let deg = 0; deg < 360; deg += 10) {
    const a = compassToCanvas(deg);
    const isMajor = deg % 30 === 0;
    const tickLen = isMajor ? 12 : 6;
    const x0 = cx + Math.cos(a) * (r - tickLen);
    const y0 = cy + Math.sin(a) * (r - tickLen);
    const x1 = cx + Math.cos(a) * r;
    const y1 = cy + Math.sin(a) * r;

    ctx.beginPath();
    ctx.moveTo(x0, y0);
    ctx.lineTo(x1, y1);
    ctx.strokeStyle = isMajor ? "#8a9fc0" : "#3d5070";
    ctx.lineWidth = isMajor ? 2 : 1;
    ctx.stroke();

    if (compassLabels[deg] !== undefined) {
      const lx = cx + Math.cos(a) * (r - 26);
      const ly = cy + Math.sin(a) * (r - 26);
      ctx.fillStyle = "#dbe8ff";
      ctx.font = "bold 13px Trebuchet MS";
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      ctx.fillText(compassLabels[deg], lx, ly);
    }
  }

  // --- End-stop markers at North (0/360) and East (90/450) ---
  const stops = [
    { compassDeg: 0,  label: "0°" },
    { compassDeg: 90, label: "450°" },
  ];
  for (const stop of stops) {
    const a = compassToCanvas(stop.compassDeg);
    const sx = cx + Math.cos(a) * r;
    const sy = cy + Math.sin(a) * r;
    ctx.beginPath();
    ctx.arc(sx, sy, 5, 0, Math.PI * 2);
    ctx.fillStyle = "#ff6b35";
    ctx.fill();
    const lx = cx + Math.cos(a) * (r + 14);
    const ly = cy + Math.sin(a) * (r + 14);
    ctx.fillStyle = "#ff6b35";
    ctx.font = "11px Trebuchet MS";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.fillText(stop.label, lx, ly);
  }

  // --- Needle ---
  // Color: green in normal range, orange when in 360-450 overlap zone, red near limits
  const hz = state.hardwareAzDeg;
  let needleColor;
  if (hz > 440 || hz < 5) {
    needleColor = "#ff4444";
  } else if (hz > 360) {
    needleColor = "#ffaa00";
  } else {
    needleColor = "#36c9a7";
  }

  const compassDirDeg = hz % 360;
  const azA = compassToCanvas(compassDirDeg);
  const tx = cx + Math.cos(azA) * (r - 18);
  const ty = cy + Math.sin(azA) * (r - 18);

  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.lineTo(tx, ty);
  ctx.strokeStyle = needleColor;
  ctx.lineWidth = 4;
  ctx.stroke();

  ctx.beginPath();
  ctx.arc(cx, cy, 5, 0, Math.PI * 2);
  ctx.fillStyle = needleColor;
  ctx.fill();

  // --- Center text ---
  const overlapZone = hz > 360;
  ctx.fillStyle = overlapZone ? "#ffaa00" : "#dbe8ff";
  ctx.font = "bold 15px Trebuchet MS";
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillText(`AZ ${hz.toFixed(1)}°`, cx, cy - 8);
  if (overlapZone) {
    ctx.fillStyle = "#ff9933";
    ctx.font = "11px Trebuchet MS";
    ctx.fillText("OVERLAP ZONE", cx, cy + 10);
  }
}

function updateReadouts() {
  ui.ctrlAz.textContent = formatDeg(state.controllerReadoutAzDeg);
  ui.ctrlTarget.textContent = formatDeg(state.targetAzDeg);
  ui.ctrlMode.textContent = state.connected ? state.mode : "DISCONNECTED";

  ui.hwAz.textContent = formatDeg(state.hardwareAzDeg);
  ui.hwSpeed.textContent = `${state.currentSpeedDegPerSec.toFixed(1)} deg/s`;
  ui.hwDir.textContent = state.connected ? state.hardwareDirection : "N/A";
}

function frame() {
  updateReadouts();
  drawDial();

  requestAnimationFrame(frame);
}

requestAnimationFrame(frame);

setInterval(pollState, 50);
pollState();
