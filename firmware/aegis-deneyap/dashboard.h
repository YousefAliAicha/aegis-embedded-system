#pragma once
#include <pgmspace.h>

// ─────────────────────────────────────────────────────────────────────
//  AEGIS Dashboard — self-contained HTML/CSS/JS
//  Served from ESP32 flash via HTTP GET /
//  WebSocket connects to ws://<device-ip>:81
// ─────────────────────────────────────────────────────────────────────

const char DASHBOARD_HTML[] PROGMEM = R"HTMLEOF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>AEGIS · Radar Monitor</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=JetBrains+Mono:wght@300;400;600&display=swap" rel="stylesheet">
<style>
/* ── Reset ───────────────────────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

/* ── Design tokens ───────────────────────────────────────── */
:root {
  --bg:           #050D1A;
  --panel:        #070F1E;
  --panel2:       #08111F;
  --border:       rgba(0,229,255,0.18);
  --border-bright:rgba(0,229,255,0.45);
  --cyan:         #00E5FF;
  --cyan-dim:     rgba(0,229,255,0.35);
  --cyan-glow:    rgba(0,229,255,0.08);
  --cyan-faint:   rgba(0,229,255,0.04);
  --amber:        #FFB300;
  --amber-dim:    rgba(255,179,0,0.3);
  --red:          #FF3355;
  --red-dim:      rgba(255,51,85,0.15);
  --green:        #00FF88;
  --text:         #A8C4CC;
  --text-bright:  #D4EEF2;
  --text-dim:     #3A5560;
  --label:        #4A7080;
}

/* ── Body & grid texture ─────────────────────────────────── */
body {
  background: var(--bg);
  color: var(--text);
  font-family: 'Share Tech Mono', 'JetBrains Mono', monospace;
  font-size: 12px;
  min-height: 100vh;
  overflow: hidden;
  background-image:
    linear-gradient(rgba(0,229,255,0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(0,229,255,0.03) 1px, transparent 1px);
  background-size: 40px 40px;
}

.bracketed { position: relative; }
.bracketed::before, .bracketed::after {
  content: '';
  position: absolute;
  width: 10px; height: 10px;
  border-color: var(--border-bright);
  border-style: solid;
  pointer-events: none;
}
.bracketed::before { top: 0; left: 0;  border-width: 1px 0 0 1px; }
.bracketed::after  { top: 0; right: 0; border-width: 1px 1px 0 0; }

/* ── Shell ───────────────────────────────────────────────── */
.shell {
  display: grid;
  grid-template-rows: 44px 1fr;
  height: 100vh;
  overflow: hidden;
}

/* ── Header ──────────────────────────────────────────────── */
header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 18px;
  border-bottom: 1px solid var(--border);
  background: var(--panel);
  flex-shrink: 0;
}
.logo { display: flex; align-items: baseline; gap: 10px; }
.logo-name {
  font-family: 'Share Tech Mono', monospace;
  font-size: 17px;
  letter-spacing: 0.25em;
  color: var(--cyan);
  text-shadow: 0 0 12px rgba(0,229,255,0.5);
}
.logo-sub { font-size: 9px; letter-spacing: 0.18em; color: var(--text-dim); }
.header-center { display: flex; align-items: center; gap: 24px; }
.hdr-stat { display: flex; align-items: center; gap: 6px; }
.hdr-stat-label { font-size: 9px; letter-spacing: 0.15em; color: var(--text-dim); }
.hdr-stat-val { font-size: 13px; color: var(--text-bright); min-width: 48px; }
.conn-wrap { display: flex; align-items: center; gap: 7px; }
.conn-dot { width: 7px; height: 7px; border-radius: 50%; background: var(--text-dim); transition: background 0.3s, box-shadow 0.3s; }
.conn-dot.live { background: var(--green); box-shadow: 0 0 8px var(--green); }
.conn-dot.dead { background: var(--red); }
.conn-label { font-size: 9px; letter-spacing: 0.15em; color: var(--text-dim); }

/* ── Main 3-column layout ────────────────────────────────── */
.main {
  display: grid;
  grid-template-columns: 260px 1fr 280px;
  height: calc(100vh - 44px);
  overflow: hidden;
}

.panel { border-right: 1px solid var(--border); display: flex; flex-direction: column; overflow: hidden; }
.panel:last-child { border-right: none; }
.panel-header {
  padding: 7px 14px;
  border-bottom: 1px solid var(--border);
  font-size: 9px;
  letter-spacing: 0.22em;
  color: var(--label);
  flex-shrink: 0;
  background: var(--panel);
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.panel-header-accent { width: 4px; height: 4px; background: var(--cyan); border-radius: 50%; box-shadow: 0 0 6px var(--cyan); }

/* ── Left panel ──────────────────────────────────────────── */
.left-panel {
  background: var(--panel);
  overflow-y: auto;
  scrollbar-width: thin;
  scrollbar-color: var(--border) transparent;
}
.left-panel::-webkit-scrollbar { width: 3px; }
.left-panel::-webkit-scrollbar-thumb { background: var(--border); }

.status-block { padding: 12px 14px; border-bottom: 1px solid var(--border); }
.status-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 6px; }
.status-row:last-child { margin-bottom: 0; }
.s-label { font-size: 9px; letter-spacing: 0.15em; color: var(--label); }
.s-val { font-size: 11px; color: var(--text-bright); }
.s-val.online  { color: var(--green); }
.s-val.offline { color: var(--red); }
.s-val.warn    { color: var(--amber); }

.alert-banner {
  margin: 10px 14px;
  padding: 8px 10px;
  border: 1px solid var(--border);
  border-radius: 2px;
  font-size: 10px;
  letter-spacing: 0.12em;
  text-align: center;
  transition: all 0.25s;
  position: relative;
}
.alert-banner::before, .alert-banner::after {
  content: '';
  position: absolute;
  width: 8px; height: 8px;
  border-color: currentColor;
  border-style: solid;
  opacity: 0.6;
}
.alert-banner::before { bottom: 0; left: 0;  border-width: 0 0 1px 1px; }
.alert-banner::after  { bottom: 0; right: 0; border-width: 0 1px 1px 0; }
.alert-banner.clear  { color: var(--green);  border-color: rgba(0,255,136,0.25); background: rgba(0,255,136,0.04); }
.alert-banner.threat { color: var(--red);    border-color: rgba(255,51,85,0.4);  background: var(--red-dim); animation: threat-pulse 0.7s ease-in-out infinite; }
.alert-banner.breach { color: var(--amber);  border-color: var(--amber-dim);     background: rgba(255,179,0,0.06); animation: threat-pulse 0.7s ease-in-out infinite; }
@keyframes threat-pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.65; } }

.ctrl-block { padding: 10px 14px; border-bottom: 1px solid var(--border); position: relative; }
.ctrl-title {
  font-size: 9px;
  letter-spacing: 0.2em;
  color: var(--label);
  margin-bottom: 8px;
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.ctrl-title-vals { font-size: 9px; letter-spacing: 0.05em; color: var(--cyan); opacity: 0.8; font-weight: normal; }
.range-ticks { display: flex; justify-content: space-between; margin-top: 3px; padding: 0 2px; font-size: 8px; color: var(--text-dim); letter-spacing: 0.05em; }
.ctrl-row { display: flex; align-items: center; gap: 8px; margin-bottom: 7px; }
.ctrl-row:last-child { margin-bottom: 0; }
.ctrl-lbl { font-size: 9px; color: var(--text-dim); min-width: 68px; letter-spacing: 0.08em; }
input[type="range"] {
  flex: 1;
  -webkit-appearance: none;
  height: 2px;
  background: var(--border);
  border-radius: 1px;
  outline: none;
  cursor: pointer;
}
input[type="range"]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 10px; height: 10px;
  border-radius: 50%;
  background: var(--cyan);
  border: 2px solid var(--bg);
  box-shadow: 0 0 4px var(--cyan);
}
input[type="range"]::-moz-range-thumb {
  width: 10px; height: 10px;
  border-radius: 50%;
  background: var(--cyan);
  border: 2px solid var(--bg);
}
.ctrl-val { font-size: 10px; color: var(--cyan); min-width: 36px; text-align: right; }

.mode-btn {
  width: 100%;
  padding: 6px 10px;
  border: 1px solid var(--border);
  background: transparent;
  color: var(--text-dim);
  font-family: 'Share Tech Mono', monospace;
  font-size: 9px;
  letter-spacing: 0.15em;
  cursor: pointer;
  border-radius: 2px;
  display: flex;
  align-items: center;
  gap: 7px;
  transition: all 0.2s;
  margin-bottom: 5px;
}
.mode-btn:last-child { margin-bottom: 0; }
.mode-btn:hover { border-color: var(--cyan-dim); color: var(--cyan); }
.mode-btn.active { border-color: var(--cyan); color: var(--cyan); background: var(--cyan-glow); }
.mode-btn .dot { width: 6px; height: 6px; border-radius: 50%; background: var(--text-dim); flex-shrink: 0; transition: background 0.2s, box-shadow 0.2s; }
.mode-btn.active .dot { background: var(--cyan); box-shadow: 0 0 5px var(--cyan); }

.toggle-row { display: flex; align-items: center; gap: 8px; cursor: pointer; user-select: none; }
.pill { width: 26px; height: 13px; background: var(--border); border-radius: 7px; position: relative; transition: background 0.2s; flex-shrink: 0; }
.pill::after { content: ''; position: absolute; top: 2px; left: 2px; width: 9px; height: 9px; border-radius: 50%; background: var(--text-dim); transition: left 0.2s, background 0.2s; }
.toggle-row.on .pill { background: rgba(0,229,255,0.25); }
.toggle-row.on .pill::after { left: 15px; background: var(--cyan); box-shadow: 0 0 4px var(--cyan); }
.toggle-lbl { font-size: 9px; letter-spacing: 0.12em; color: var(--text-dim); }

.peri-btns { display: flex; gap: 6px; }
.peri-btn {
  flex: 1;
  padding: 5px 0;
  border: 1px solid var(--border);
  background: transparent;
  color: var(--text-dim);
  font-family: 'Share Tech Mono', monospace;
  font-size: 9px;
  letter-spacing: 0.12em;
  cursor: pointer;
  border-radius: 2px;
  transition: all 0.2s;
}
.peri-btn:hover { border-color: var(--cyan-dim); color: var(--cyan); }
.peri-btn.drawing { border-color: var(--cyan); color: var(--cyan); background: var(--cyan-glow); }
.peri-btn.clr:hover { border-color: rgba(255,51,85,0.4); color: var(--red); }

/* ── Center panel ────────────────────────────────────────── */
.center-panel {
  display: grid;
  grid-template-rows: 1fr 130px;
  overflow: hidden;
  background: var(--bg);
  border-right: 1px solid var(--border);
}
.radar-wrap {
  position: relative;
  display: flex;
  justify-content: center;
  align-items: center;
  overflow: hidden;
  padding: 16px;
}
#radarCanvas {
  position: relative;
  display: block;
  max-width: 100%;
  max-height: 100%;
  cursor: default;
  z-index: 1;
  background: transparent;
}
#radarMapBg {
  position: absolute;
  top: 0; left: 0;
  width: 100%; height: 100%;
  object-fit: cover;
  object-position: center 40%;
  opacity: 0.28;
  pointer-events: none;
  z-index: 0;
  /* clip to the semicircle area — subtle vignette handled by canvas */
  mask-image: radial-gradient(ellipse 90% 85% at 50% 100%, black 55%, transparent 100%);
  -webkit-mask-image: radial-gradient(ellipse 90% 85% at 50% 100%, black 55%, transparent 100%);
}
.manual-badge {
  display: none;
  position: absolute;
  top: 20px; left: 50%;
  transform: translateX(-50%);
  font-size: 9px;
  letter-spacing: 0.18em;
  color: var(--cyan);
  background: var(--panel);
  padding: 2px 8px;
  border: 1px solid var(--cyan-dim);
  border-radius: 2px;
  z-index: 2;
  pointer-events: none;
}
.manual-badge.visible { display: block; }

/* Waterfall */
.waterfall-wrap {
  display: flex;
  flex-direction: column;
  padding: 6px 14px 8px;
  gap: 4px;
  border-top: 1px solid var(--border);
  background: var(--panel);
  overflow: hidden;
}
.wf-label { font-size: 9px; letter-spacing: 0.2em; color: var(--label); flex-shrink: 0; display: flex; justify-content: space-between; }
.wf-label span { color: var(--text-dim); font-size: 8px; letter-spacing: 0.1em; }
#waterfallCanvas { display: block; width: 100%; flex: 1 1 auto; border: 1px solid var(--border); image-rendering: pixelated; min-height: 0; }

/* ── Right panel ─────────────────────────────────────────── */
.right-panel { background: var(--panel); display: flex; flex-direction: column; overflow: hidden; }
.tab-bar { display: flex; border-bottom: 1px solid var(--border); flex-shrink: 0; }
.tab {
  flex: 1;
  padding: 8px 0;
  text-align: center;
  font-size: 9px;
  letter-spacing: 0.18em;
  color: var(--text-dim);
  cursor: pointer;
  border-right: 1px solid var(--border);
  transition: color 0.2s, background 0.2s;
  position: relative;
}
.tab:last-child { border-right: none; }
.tab:hover { color: var(--cyan); }
.tab.active { color: var(--cyan); background: var(--cyan-faint); border-bottom: 2px solid var(--cyan); }
.tab-badge { display: none; position: absolute; top: 4px; right: 8px; background: var(--red); color: white; font-size: 8px; padding: 0 4px; border-radius: 8px; min-width: 14px; text-align: center; line-height: 14px; }
.tab-badge.visible { display: block; }
.tab-content { display: none; flex-direction: column; flex: 1; overflow: hidden; min-height: 0; }
.tab-content.active { display: flex; }

.coord-block { padding: 12px 14px; border-bottom: 1px solid var(--border); flex-shrink: 0; }
.coord-title { font-size: 9px; letter-spacing: 0.2em; color: var(--label); margin-bottom: 8px; }
.coord-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
.coord-lbl { font-size: 8px; letter-spacing: 0.15em; color: var(--text-dim); margin-bottom: 2px; }
.coord-val { font-size: 22px; font-weight: 600; color: var(--text-bright); line-height: 1; }
.coord-unit { font-size: 10px; color: var(--text-dim); margin-left: 2px; }

.lock-block { padding: 8px 14px; border-bottom: 1px solid var(--border); flex-shrink: 0; }
.lock-status { padding: 7px 10px; border: 1px solid var(--border); border-radius: 2px; font-size: 10px; letter-spacing: 0.14em; text-align: center; color: var(--text-dim); transition: all 0.25s; }
.lock-status.locked   { color: var(--red);   border-color: rgba(255,51,85,0.35); background: var(--red-dim); }
.lock-status.tracking { color: var(--amber); border-color: var(--amber-dim); background: rgba(255,179,0,0.05); }
.lock-status.clear    { color: var(--green); border-color: rgba(0,255,136,0.2); background: rgba(0,255,136,0.03); }

.signal-block { padding: 8px 14px; border-bottom: 1px solid var(--border); flex-shrink: 0; }
.signal-title { font-size: 9px; letter-spacing: 0.2em; color: var(--label); margin-bottom: 5px; }
#signalCanvas { display: block; width: 100%; height: 50px; border: 1px solid var(--border); border-radius: 2px; background: var(--bg); }

.motion-block { padding: 8px 14px; border-bottom: 1px solid var(--border); display: flex; align-items: center; gap: 8px; flex-shrink: 0; }
.motion-lbl { font-size: 9px; letter-spacing: 0.12em; color: var(--label); white-space: nowrap; }
.motion-bar-wrap { flex: 1; height: 3px; background: var(--border); border-radius: 1px; overflow: hidden; }
.motion-bar { height: 100%; width: 0%; background: var(--cyan); border-radius: 1px; transition: width 0.3s, background 0.3s; }
.motion-val { font-size: 9px; color: var(--text); min-width: 72px; text-align: right; }

.globe-block { padding: 10px 14px; display: flex; flex-direction: column; align-items: center; gap: 5px; flex-shrink: 0; border-bottom: 1px solid var(--border); }
.globe-title { font-size: 9px; letter-spacing: 0.2em; color: var(--label); align-self: flex-start; }
#globeCanvas { display: block; border: 1px solid var(--border); border-radius: 50%; background: var(--bg); }

.telemetry-block { padding: 10px 14px; display: flex; flex-direction: column; gap: 6px; flex-shrink: 0; border-bottom: 1px solid var(--border); }
.telemetry-title { font-size: 9px; letter-spacing: 0.2em; color: var(--label); margin-bottom: 2px; }
.telem-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 5px 8px; }
.telem-lbl { font-size: 8px; letter-spacing: 0.1em; color: var(--text-dim); margin-bottom: 1px; }
.telem-val { font-size: 11px; color: var(--cyan); letter-spacing: 0.05em; }
.telem-val.warn { color: var(--amber); }
.telem-val.ok   { color: var(--green); }
.telem-bar-row { display: flex; align-items: center; gap: 6px; margin-top: 2px; }
.telem-bar-lbl { font-size: 8px; color: var(--text-dim); min-width: 38px; letter-spacing: 0.08em; }
.telem-bar-wrap { flex: 1; height: 2px; background: var(--border); border-radius: 1px; }
.telem-bar { height: 100%; border-radius: 1px; transition: width 1s; }
.telem-bar.cpu { background: var(--cyan); }
.telem-bar.mem { background: var(--amber); }
.telem-bar-num { font-size: 8px; color: var(--text-dim); min-width: 28px; text-align: right; }

.log-controls { padding: 6px 14px; border-bottom: 1px solid var(--border); display: flex; align-items: center; justify-content: space-between; flex-shrink: 0; }
.log-count { font-size: 9px; color: var(--text-dim); letter-spacing: 0.1em; }
.clr-btn { font-size: 9px; color: var(--text-dim); background: none; border: none; cursor: pointer; letter-spacing: 0.12em; font-family: 'Share Tech Mono', monospace; padding: 0; }
.clr-btn:hover { color: var(--red); }
#alertLog { flex: 1 1 auto; overflow-y: auto; padding: 4px 0; scrollbar-width: thin; scrollbar-color: var(--border) transparent; min-height: 0; }
#alertLog::-webkit-scrollbar { width: 3px; }
#alertLog::-webkit-scrollbar-thumb { background: var(--border); }
.log-entry { padding: 4px 14px; font-size: 9px; line-height: 1.65; border-left: 2px solid transparent; letter-spacing: 0.05em; }
.log-entry.inner { color: var(--red); border-left-color: var(--red); background: rgba(255,51,85,0.03); }
.log-entry.outer { color: var(--amber); border-left-color: var(--amber); background: rgba(255,179,0,0.03); }
.log-ts { color: var(--text-dim); margin-right: 5px; }
.log-zone { font-size: 8px; padding: 0 4px; border-radius: 2px; margin-right: 4px; }
.log-entry.inner .log-zone { background: rgba(255,51,85,0.2); color: var(--red); }
.log-entry.outer .log-zone { background: rgba(255,179,0,0.15); color: var(--amber); }
</style>
</head>
<body>
<div class="shell">

  <!-- ── Header ── -->
  <header>
    <div class="logo">
      <span class="logo-name">AEGIS</span>
      <span class="logo-sub">RADAR MONITOR v1.0</span>
    </div>
    <div class="header-center">
      <div class="hdr-stat">
        <span class="hdr-stat-label">ANGLE</span>
        <span class="hdr-stat-val" id="hdrAngle">--°</span>
      </div>
      <div class="hdr-stat">
        <span class="hdr-stat-label">DIST</span>
        <span class="hdr-stat-val" id="hdrDist">-- cm</span>
      </div>
    </div>
    <div class="conn-wrap">
      <div class="conn-dot" id="connDot"></div>
      <span class="conn-label" id="connLabel">OFFLINE</span>
    </div>
  </header>

  <!-- ── Main ── -->
  <div class="main">

    <!-- ════ LEFT PANEL ════ -->
    <div class="panel left-panel">
      <div class="panel-header">System Status<div class="panel-header-accent"></div></div>

      <div class="status-block">
        <div class="status-row"><span class="s-label">CONNECTION</span><span class="s-val offline" id="sConn">OFFLINE</span></div>
        <div class="status-row"><span class="s-label">SWEEP MODE</span><span class="s-val" id="sMode">AUTO</span></div>
        <div class="status-row"><span class="s-label">ZONE</span><span class="s-val" id="sZone">--</span></div>
      </div>

      <div class="alert-banner clear" id="alertBanner">AIRSPACE CLEAR</div>

      <div class="ctrl-block">
        <div class="ctrl-title">Sweep Arc <span class="ctrl-title-vals" id="sweepArcSummary">0° – 180°</span></div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Min angle</span>
          <input type="range" id="sliderSweepMin" min="0" max="179" value="0" oninput="onSweepMin(this.value)">
          <span class="ctrl-val" id="valSweepMin">0°</span>
        </div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Max angle</span>
          <input type="range" id="sliderSweepMax" min="1" max="180" value="180" oninput="onSweepMax(this.value)">
          <span class="ctrl-val" id="valSweepMax">180°</span>
        </div>
      </div>

      <div class="ctrl-block">
        <div class="ctrl-title">Sensor Tuning <span class="ctrl-title-vals" id="sensorSummary">25ms · 2° · 40cm</span></div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Speed</span>
          <input type="range" id="sliderDelay" min="5" max="150" value="25" oninput="onStepDelay(this.value)">
          <span class="ctrl-val" id="valDelay">25ms</span>
        </div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Step size</span>
          <input type="range" id="sliderStep" min="1" max="10" value="2" oninput="onStepDeg(this.value)">
          <span class="ctrl-val" id="valStep">2°</span>
        </div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Threshold</span>
          <input type="range" id="sliderThreshold" min="5" max="200" value="40" oninput="onThreshold(this.value)">
          <span class="ctrl-val" id="valThreshold">40cm</span>
        </div>
      </div>

      <div class="ctrl-block">
        <div class="ctrl-title">Steer Mode</div>
        <button class="mode-btn" id="manualToggle" onclick="toggleManual()">
          <div class="dot"></div>MANUAL STEER
        </button>
      </div>

      <div class="ctrl-block">
        <div class="ctrl-title">Threat Perimeter</div>
        <div style="font-size:9px;color:var(--text-dim);margin-bottom:7px;line-height:1.55;">
          Click Draw, then click points on the radar to outline a zone. Detections inside trigger a breach alarm.
        </div>
        <div class="peri-btns">
          <button class="peri-btn" id="periDrawBtn" onclick="togglePerimeterDraw()">DRAW</button>
          <button class="peri-btn clr" onclick="clearPerimeter()">CLEAR</button>
        </div>
      </div>

      <div class="ctrl-block">
        <div class="ctrl-title">Audio Engine</div>
        <div class="toggle-row" id="audioToggle" onclick="toggleAudio()">
          <div class="pill"></div>
          <span class="toggle-lbl">SONAR PING</span>
        </div>
      </div>

      <div class="ctrl-block">
        <div class="ctrl-title">Radar Range <span class="ctrl-title-vals" id="radarRangeSummary">100cm</span></div>
        <div class="ctrl-row">
          <span class="ctrl-lbl">Range</span>
          <input type="range" id="sliderRadarRange" min="20" max="220" value="100" oninput="onRadarRange(this.value)">
          <span class="ctrl-val" id="valRadarRange">100cm</span>
        </div>
        <div class="range-ticks"><span>20</span><span>80</span><span>140</span><span>220</span></div>
      </div>

    </div><!-- /left-panel -->

    <!-- ════ CENTER PANEL ════ -->
    <div class="center-panel">
      <div class="radar-wrap" id="radarWrap">
        <img id="radarMapBg" alt="" src="data:image/webp;base64,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">
        <canvas id="radarCanvas"></canvas>
        <div class="manual-badge" id="manualBadge">⊕ MANUAL STEER ACTIVE</div>
      </div>
      <div class="waterfall-wrap">
        <div class="wf-label">
          History · Waterfall
          <span>← time · distance →</span>
        </div>
        <canvas id="waterfallCanvas" height="80"></canvas>
      </div>
    </div>

    <!-- ════ RIGHT PANEL ════ -->
    <div class="panel right-panel">
      <div class="tab-bar">
        <div class="tab active" id="tabLive" onclick="switchTab('live')">Live</div>
        <div class="tab" id="tabLog" onclick="switchTab('log')">
          Alert Log
          <span class="tab-badge" id="alertBadge">0</span>
        </div>
      </div>

      <div class="tab-content active" id="contentLive">
        <div class="coord-block">
          <div class="coord-title">Last Target Fix</div>
          <div class="coord-grid">
            <div class="coord-item">
              <div class="coord-lbl">BEARING</div>
              <div class="coord-val" id="coordAngle">--<span class="coord-unit">°</span></div>
            </div>
            <div class="coord-item">
              <div class="coord-lbl">RANGE</div>
              <div class="coord-val" id="coordDist">--<span class="coord-unit">cm</span></div>
            </div>
          </div>
        </div>

        <div class="lock-block">
          <div class="lock-status clear" id="lockStatus">NO TARGET</div>
        </div>

        <div class="signal-block">
          <div class="signal-title">Signal Waveform</div>
          <canvas id="signalCanvas"></canvas>
        </div>

        <div class="motion-block">
          <span class="motion-lbl">Motion</span>
          <div class="motion-bar-wrap"><div class="motion-bar" id="motionBar"></div></div>
          <span class="motion-val" id="motionVal">—</span>
        </div>

        <div class="globe-block">
          <div class="globe-title">Global Position</div>
          <canvas id="globeCanvas" width="180" height="180"></canvas>
        </div>

        <div class="telemetry-block">
          <div class="telemetry-title">System Telemetry</div>
          <div class="telem-grid">
            <div class="telem-item">
              <div class="telem-lbl">LAT / LON</div>
              <div class="telem-val" id="telemPos">35.2°N 23.4°E</div>
            </div>
            <div class="telem-item">
              <div class="telem-lbl">UPTIME</div>
              <div class="telem-val ok" id="telemUptime">00:00:00</div>
            </div>
            <div class="telem-item">
              <div class="telem-lbl">TEMP</div>
              <div class="telem-val" id="telemTemp">--°C</div>
            </div>
            <div class="telem-item">
              <div class="telem-lbl">SCANS</div>
              <div class="telem-val" id="telemScans">0</div>
            </div>
          </div>
          <div class="telem-bar-row">
            <span class="telem-bar-lbl">CPU</span>
            <div class="telem-bar-wrap"><div class="telem-bar cpu" id="cpuBar" style="width:12%"></div></div>
            <span class="telem-bar-num" id="cpuPct">12%</span>
          </div>
          <div class="telem-bar-row">
            <span class="telem-bar-lbl">MEM</span>
            <div class="telem-bar-wrap"><div class="telem-bar mem" id="memBar" style="width:34%"></div></div>
            <span class="telem-bar-num" id="memPct">34%</span>
          </div>
        </div>
      </div><!-- /live tab -->

      <div class="tab-content" id="contentLog">
        <div class="log-controls">
          <span class="log-count" id="logCount">0 events</span>
          <button class="clr-btn" onclick="clearLog()">CLR</button>
        </div>
        <div id="alertLog"></div>
      </div>
    </div><!-- /right-panel -->
  </div><!-- /main -->
</div><!-- /shell -->

<script>
// ════════════════════════════════════════════════════════════════
//  AEGIS Dashboard — client-side
// ════════════════════════════════════════════════════════════════

// ── WebSocket ──────────────────────────────────────────────────
const WS_HOST = location.hostname || '192.168.1.1';
const WS_URL  = `ws://${WS_HOST}:81`;
let ws = null, wsReconnectT = null;
const WS_RETRY_MS = 3000;

function wsConnect() {
  ws = new WebSocket(WS_URL);
  ws.onopen    = () => { clearTimeout(wsReconnectT); setConnStatus(true); };
  ws.onmessage = (ev) => { try { processFrame(JSON.parse(ev.data)); } catch(e) {} };
  ws.onclose = ws.onerror = () => { setConnStatus(false); wsReconnectT = setTimeout(wsConnect, WS_RETRY_MS); };
}
function wsSend(obj) {
  if (ws && ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(obj));
}
function setConnStatus(live) {
  document.getElementById('connDot').className     = 'conn-dot ' + (live ? 'live' : 'dead');
  document.getElementById('connLabel').textContent = live ? 'LIVE' : 'OFFLINE';
  document.getElementById('sConn').textContent     = live ? 'ONLINE' : 'OFFLINE';
  document.getElementById('sConn').className       = 's-val ' + (live ? 'online' : 'offline');
}

// ── Radar canvas ───────────────────────────────────────────────
const radarCanvas = document.getElementById('radarCanvas');
const rctx        = radarCanvas.getContext('2d');

// R, CX, CY are always in LOGICAL (CSS) pixels — DPR is only applied via ctx.scale at resize
let R = 0, CX = 0, CY = 0;
let dpr = 1;

const INNER_ZONE_CM = 50;
let   MAX_RANGE_CM  = 100;

// servoToRad: servo 0°=left, 180°=right
// maps to canvas angle: 0° servo → π rad (pointing left), 180° → 0 rad (pointing right)
function servoToRad(a) { return (180 - a) * Math.PI / 180; }

function resizeRadar() {
  dpr = window.devicePixelRatio || 1;
  const wrap   = document.getElementById('radarWrap');
  const availW = wrap.clientWidth  - 32;
  const availH = wrap.clientHeight - 28;
  // size is the full diameter; radar is a semicircle so height = size/2 + margin
  const size   = Math.min(availW, availH * 2);

  // Logical dimensions
  const logW = size;
  const logH = Math.floor(size / 2) + 46;

  // CSS size
  radarCanvas.style.width  = logW + 'px';
  radarCanvas.style.height = logH + 'px';

  // Physical size (sharp on HiDPI)
  radarCanvas.width  = Math.round(logW  * dpr);
  radarCanvas.height = Math.round(logH * dpr);

  // Reset transform and apply DPR scale once
  rctx.setTransform(dpr, 0, 0, dpr, 0, 0);

  // All geometry in logical pixels from here on
  R  = size / 2 - 38;
  CX = size / 2;
  CY = Math.floor(size / 2) + 38;
}

// Trail buffer
const TRAIL_LEN = 30;
const trail = [];

function _pushTrailInner(angle, dist, det) {
  trail.push({ angle, dist, det });
  if (trail.length > TRAIL_LEN * 45) trail.splice(0, 20);
  scanCount++;
  const el = document.getElementById('telemScans');
  if (el) el.textContent = scanCount;
}

let currentAngle = 0, currentDist = -1, currentDet = false;
let sweepMinDeg = 0, sweepMaxDeg = 180, thresholdCm = 40;

function drawRadar() {
  // All drawing coordinates are in logical pixels (rctx is pre-scaled by DPR)
  rctx.clearRect(0, 0, radarCanvas.width / dpr, radarCanvas.height / dpr);

  // Vignette
  const vigGrad = rctx.createRadialGradient(CX, CY, R * 0.7, CX, CY, R * 1.4);
  vigGrad.addColorStop(0, 'rgba(5,13,26,0)');
  vigGrad.addColorStop(1, 'rgba(5,13,26,0.85)');
  rctx.save();
  rctx.fillStyle = vigGrad;
  rctx.fillRect(0, 0, radarCanvas.width / dpr, radarCanvas.height / dpr);
  rctx.restore();

  // ── Inner zone fill (proportional to MAX_RANGE_CM) ──────────
  const innerR = R * (INNER_ZONE_CM / MAX_RANGE_CM);
  rctx.save();
  rctx.globalAlpha = 0.04;
  rctx.fillStyle = '#00E5FF';
  rctx.beginPath();
  rctx.arc(CX, CY, innerR, Math.PI, 0);
  rctx.lineTo(CX, CY);
  rctx.closePath();
  rctx.fill();
  rctx.restore();

  // ── Threshold ring — drawn on main radar, scaled by MAX_RANGE_CM ──
  // FIX: threshold ring radius = (thresholdCm / MAX_RANGE_CM) * R
  const threshR = R * Math.min(thresholdCm, MAX_RANGE_CM) / MAX_RANGE_CM;
  rctx.save();
  rctx.strokeStyle = 'rgba(255,179,0,0.75)';
  rctx.lineWidth = 1.5;
  rctx.setLineDash([4, 4]);
  rctx.beginPath();
  rctx.arc(CX, CY, threshR, Math.PI, 0);
  rctx.stroke();
  rctx.setLineDash([]);
  // Label above the ring at 90°
  rctx.fillStyle = 'rgba(255,179,0,0.9)';
  rctx.font = '9px "Share Tech Mono",monospace';
  rctx.textAlign = 'center';
  rctx.fillText(`${thresholdCm}cm`, CX, CY - threshR - 4);
  rctx.restore();

  // ── Perimeter threat zone fill ───────────────────────────────
  if (perimeterPoints.length > 2) {
    rctx.save();
    rctx.globalAlpha = 0.07;
    rctx.fillStyle = '#FFB300';
    rctx.beginPath();
    perimeterPoints.forEach((p, i) => i === 0 ? rctx.moveTo(p.x, p.y) : rctx.lineTo(p.x, p.y));
    rctx.closePath();
    rctx.fill();
    rctx.restore();
    rctx.save();
    rctx.strokeStyle = '#FFB300';
    rctx.lineWidth = 1;
    rctx.globalAlpha = 0.4;
    rctx.setLineDash([3, 5]);
    rctx.beginPath();
    perimeterPoints.forEach((p, i) => i === 0 ? rctx.moveTo(p.x, p.y) : rctx.lineTo(p.x, p.y));
    rctx.closePath();
    rctx.stroke();
    rctx.setLineDash([]);
    rctx.restore();
  }

  // ── Range rings ──────────────────────────────────────────────
  const ringStep = MAX_RANGE_CM <= 60 ? 10 : MAX_RANGE_CM <= 120 ? 20 : 50;
  for (let cm = ringStep; cm <= MAX_RANGE_CM; cm += ringStep) {
    const rr = R * (cm / MAX_RANGE_CM);
    const isEdge = cm === MAX_RANGE_CM;
    rctx.strokeStyle = isEdge ? 'rgba(0,229,255,0.45)' : 'rgba(0,229,255,0.18)';
    rctx.lineWidth = isEdge ? 1.5 : 1;
    rctx.beginPath();
    rctx.arc(CX, CY, rr, Math.PI, 0);
    rctx.stroke();
    rctx.fillStyle = isEdge ? 'rgba(0,229,255,0.9)' : 'rgba(0,229,255,0.55)';
    rctx.font = `${isEdge ? 12 : 11}px "Share Tech Mono",monospace`;
    rctx.textAlign = 'left';
    rctx.fillText(`${cm}`, CX + rr + 3, CY - 3);
  }

  // ── Angle spokes ─────────────────────────────────────────────
  for (let a = 0; a <= 180; a += 15) {
    const rad = servoToRad(a);
    const isMain = a % 45 === 0;
    rctx.strokeStyle = isMain ? 'rgba(0,229,255,0.10)' : 'rgba(0,229,255,0.05)';
    rctx.lineWidth = 1;
    rctx.beginPath();
    rctx.moveTo(CX, CY);
    rctx.lineTo(CX + R * Math.cos(rad), CY - R * Math.sin(rad));
    rctx.stroke();
    const lx = CX + (R + 14) * Math.cos(rad);
    const ly = CY - (R + 14) * Math.sin(rad);
    rctx.fillStyle = isMain ? 'rgba(0,229,255,0.75)' : 'rgba(0,229,255,0.42)';
    rctx.font = `${isMain ? 12 : 10}px "Share Tech Mono",monospace`;
    rctx.textAlign = 'center';
    rctx.fillText(`${a}°`, lx, ly + 4);
  }

  // ── Sweep arc limit markers ──────────────────────────────────
  [sweepMinDeg, sweepMaxDeg].forEach(a => {
    const rad = servoToRad(a);
    rctx.strokeStyle = 'rgba(0,229,255,0.2)';
    rctx.lineWidth = 1;
    rctx.setLineDash([2, 6]);
    rctx.beginPath();
    rctx.moveTo(CX, CY);
    rctx.lineTo(CX + R * 0.9 * Math.cos(rad), CY - R * 0.9 * Math.sin(rad));
    rctx.stroke();
    rctx.setLineDash([]);
  });

  // ── Phosphor trail ────────────────────────────────────────────
  trail.forEach((f, i) => {
    const age = trail.length - i;
    const op  = Math.max(0, 0.45 - age * 0.012);
    if (op <= 0) return;
    const rad = servoToRad(f.angle);
    const inInner = f.dist > 0 && f.dist <= INNER_ZONE_CM;
    const color   = f.det ? (inInner ? '#FF3355' : '#FFB300') : '#00E5FF';
    rctx.save();
    rctx.globalAlpha = op * 0.3;
    rctx.strokeStyle = color;
    rctx.lineWidth = inInner ? 1.5 : 1;
    rctx.beginPath();
    rctx.moveTo(CX, CY);
    rctx.lineTo(CX + R * Math.cos(rad), CY - R * Math.sin(rad));
    rctx.stroke();
    rctx.restore();
    if (f.dist > 0 && f.dist <= MAX_RANGE_CM) {
      const scale = Math.min(f.dist / MAX_RANGE_CM, 1);
      rctx.save();
      rctx.globalAlpha = op * 0.8;
      rctx.fillStyle = color;
      rctx.beginPath();
      rctx.arc(CX + R * scale * Math.cos(rad), CY - R * scale * Math.sin(rad), inInner ? 4 : 2.5, 0, Math.PI * 2);
      rctx.fill();
      rctx.restore();
    }
  });

  // ── Active sweep beam ─────────────────────────────────────────
  const activeRad = servoToRad(currentAngle);
  const inInner   = currentDist > 0 && currentDist <= INNER_ZONE_CM;
  const beamColor = currentDet ? (inInner ? '#FF3355' : '#FFB300') : '#00E5FF';

  // Beam glow fan
  rctx.save();
  rctx.globalAlpha = 0.12;
  const fan = rctx.createLinearGradient(CX, CY, CX + R * Math.cos(activeRad), CY - R * Math.sin(activeRad));
  fan.addColorStop(0, beamColor);
  fan.addColorStop(1, 'transparent');
  rctx.fillStyle = fan;
  const bw = Math.PI / 28;
  rctx.beginPath();
  rctx.moveTo(CX, CY);
  rctx.arc(CX, CY, R, -(activeRad + bw), -(activeRad - bw));
  rctx.closePath();
  rctx.fill();
  rctx.restore();

  // Beam line
  rctx.strokeStyle = beamColor;
  rctx.lineWidth   = inInner ? 2.5 : 1.5;
  rctx.shadowColor = beamColor;
  rctx.shadowBlur  = inInner ? 10 : 5;
  rctx.beginPath();
  rctx.moveTo(CX, CY);
  rctx.lineTo(CX + R * Math.cos(activeRad), CY - R * Math.sin(activeRad));
  rctx.stroke();
  rctx.shadowBlur = 0;

  // Ping dot
  if (currentDist > 0 && currentDist <= MAX_RANGE_CM) {
    const scale = Math.min(currentDist / MAX_RANGE_CM, 1);
    const dx = CX + R * scale * Math.cos(activeRad);
    const dy = CY - R * scale * Math.sin(activeRad);
    const dotR = inInner ? 7 : (currentDet ? 5 : 4);
    rctx.fillStyle   = beamColor;
    rctx.shadowColor = beamColor;
    rctx.shadowBlur  = 14;
    rctx.beginPath();
    rctx.arc(dx, dy, dotR, 0, Math.PI * 2);
    rctx.fill();
    rctx.shadowBlur = 0;
    rctx.fillStyle  = beamColor;
    rctx.font       = '10px "Share Tech Mono",monospace';
    rctx.textAlign  = 'left';
    rctx.fillText(`${Math.round(currentDist)}cm`, dx + dotR + 4, dy + 4);
  }

  // ── Base arc ──────────────────────────────────────────────────
  rctx.strokeStyle = 'rgba(0,229,255,0.3)';
  rctx.lineWidth = 2;
  rctx.beginPath();
  rctx.arc(CX, CY, R, Math.PI, 0);
  rctx.stroke();

  // ── Center pip ────────────────────────────────────────────────
  rctx.fillStyle   = '#00E5FF';
  rctx.shadowColor = '#00E5FF';
  rctx.shadowBlur  = 8;
  rctx.beginPath();
  rctx.arc(CX, CY, 3, 0, Math.PI * 2);
  rctx.fill();
  rctx.shadowBlur = 0;
}

// ── Waterfall ──────────────────────────────────────────────────
const wfCanvas = document.getElementById('waterfallCanvas');
const wfCtx    = wfCanvas.getContext('2d');

function resizeWaterfall() {
  wfCanvas.width = wfCanvas.parentElement.clientWidth - 28;
}

function waterfallPush(angle, dist, det) {
  const w = wfCanvas.width, h = wfCanvas.height;
  if (!w || !h) return;
  const img = wfCtx.getImageData(1, 0, w - 1, h);
  wfCtx.putImageData(img, 0, 0);
  const y = dist <= 0 ? h - 1 : Math.round((1 - Math.min(dist / MAX_RANGE_CM, 1)) * (h - 1));
  wfCtx.fillStyle = 'rgba(5,13,26,0.15)';
  wfCtx.fillRect(w - 1, 0, 1, h);
  let r = 0, g = 0, b = 0;
  if (dist <= 0) {
    r = 5; g = 13; b = 26;
  } else if (dist <= INNER_ZONE_CM) {
    const t = 1 - (dist / INNER_ZONE_CM);
    r = Math.round(t * 255);
    g = Math.round(229 + t * 26);
    b = 255;
  } else {
    const t = 1 - (dist / MAX_RANGE_CM);
    r = 0;
    g = Math.round(t * 160);
    b = Math.round(t * 180);
  }
  wfCtx.fillStyle = `rgb(${r},${g},${b})`;
  wfCtx.fillRect(w - 1, Math.max(0, y - 1), 1, 3);
}

// ── Signal waveform ────────────────────────────────────────────
const sigCanvas = document.getElementById('signalCanvas');
const sigCtx    = sigCanvas.getContext('2d');
const sigBuf    = [];
const SIG_LEN   = 120;

function pushSignal(dist) {
  sigBuf.push(dist);
  if (sigBuf.length > SIG_LEN) sigBuf.shift();
  const w = sigCanvas.clientWidth || 220;
  const h = 50;
  sigCanvas.width  = w;
  sigCanvas.height = h;
  sigCtx.fillStyle = '#050D1A';
  sigCtx.fillRect(0, 0, w, h);
  if (sigBuf.length < 2) return;
  sigCtx.strokeStyle = '#00E5FF';
  sigCtx.lineWidth   = 1.5;
  sigCtx.shadowColor = '#00E5FF';
  sigCtx.shadowBlur  = 3;
  sigCtx.beginPath();
  sigBuf.forEach((d, i) => {
    const x = (i / (SIG_LEN - 1)) * w;
    const y = d <= 0 ? h - 2 : h - (Math.min(d / MAX_RANGE_CM, 1) * (h - 4)) - 2;
    i === 0 ? sigCtx.moveTo(x, y) : sigCtx.lineTo(x, y);
  });
  sigCtx.stroke();
  sigCtx.shadowBlur = 0;
}

// ── Globe ──────────────────────────────────────────────────────
const globeCanvas = document.getElementById('globeCanvas');
const gctx        = globeCanvas.getContext('2d');
let   globeAngle  = 2.0;

const CONTINENTS = [
  [[-168,72],[-140,70],[-120,60],[-95,48],[-80,45],[-70,43],[-67,44],[-60,47],[-65,60],[-80,72],[-100,73],[-130,68],[-155,60],[-168,60]],
  [[-80,12],[-75,5],[-50,-5],[-35,-10],[-40,-20],[-43,-23],[-50,-30],[-53,-34],[-65,-55],[-70,-45],[-75,-35],[-80,-15],[-78,0],[-78,8]],
  [[0,50],[5,48],[10,44],[15,38],[20,37],[25,37],[30,40],[28,42],[25,47],[20,48],[15,46],[12,44],[10,47],[8,48],[2,51],[-3,51],[-5,43],[-9,39],[-10,37],[-5,36],[0,35],[5,36],[10,38],[15,37]],
  [[-5,36],[10,37],[20,35],[32,30],[42,12],[45,10],[40,0],[35,-5],[30,-20],[28,-30],[17,-35],[10,-18],[0,-5],[-5,5],[-15,10],[-17,15],[-12,30],[-5,35]],
  [[25,47],[30,42],[40,37],[45,38],[50,37],[55,25],[65,22],[75,20],[80,12],[85,20],[95,25],[100,5],[105,5],[110,15],[120,22],[130,30],[135,35],[140,45],[130,50],[120,53],[100,55],[80,65],[60,65],[40,65],[30,60],[28,57]],
  [[115,-22],[120,-18],[125,-14],[130,-12],[135,-12],[140,-17],[148,-20],[150,-25],[148,-32],[145,-38],[140,-38],[135,-35],[130,-32],[115,-30],[113,-25]],
  [[-55,60],[-45,60],[-35,65],[-20,70],[-20,80],[-40,83],[-60,80],[-65,72],[-55,60]],
];

function projectGlobe(lon, lat, angle, cx, cy, gr) {
  const lonR = (lon * Math.PI / 180) + angle;
  const latR = lat * Math.PI / 180;
  const cosLat = Math.cos(latR);
  const x3 = cosLat * Math.sin(lonR);
  const y3 = Math.sin(latR);
  const z3 = cosLat * Math.cos(lonR);
  if (z3 < 0) return null;
  return { x: cx + gr * x3, y: cy - gr * y3, z: z3 };
}

function drawGlobe() {
  const gdpr = window.devicePixelRatio || 1;
  const SIZE = 180;
  if (globeCanvas.width !== SIZE * gdpr) {
    globeCanvas.width  = SIZE * gdpr;
    globeCanvas.height = SIZE * gdpr;
    globeCanvas.style.width  = SIZE + 'px';
    globeCanvas.style.height = SIZE + 'px';
    gctx.scale(gdpr, gdpr);
  }
  const w = SIZE, h = SIZE, cx = w/2, cy = h/2, gr = 80;
  gctx.clearRect(0, 0, w, h);
  const bgGrad = gctx.createRadialGradient(cx, cy, 0, cx, cy, gr);
  bgGrad.addColorStop(0, '#0A1830');
  bgGrad.addColorStop(1, '#020810');
  gctx.fillStyle = bgGrad;
  gctx.beginPath();
  gctx.arc(cx, cy, gr, 0, Math.PI * 2);
  gctx.fill();
  const atmGrad = gctx.createRadialGradient(cx, cy, gr - 4, cx, cy, gr + 6);
  atmGrad.addColorStop(0, 'rgba(0,100,180,0.3)');
  atmGrad.addColorStop(1, 'rgba(0,100,180,0)');
  gctx.fillStyle = atmGrad;
  gctx.beginPath();
  gctx.arc(cx, cy, gr + 6, 0, Math.PI * 2);
  gctx.fill();
  gctx.save();
  gctx.beginPath();
  gctx.arc(cx, cy, gr, 0, Math.PI * 2);
  gctx.clip();
  const oceanGrad = gctx.createRadialGradient(cx - 20, cy - 20, 0, cx, cy, gr);
  oceanGrad.addColorStop(0, '#0A2540');
  oceanGrad.addColorStop(1, '#030F20');
  gctx.fillStyle = oceanGrad;
  gctx.fillRect(cx - gr, cy - gr, gr * 2, gr * 2);
  for (let lat = -80; lat <= 80; lat += 20) {
    const latR = lat * Math.PI / 180;
    gctx.strokeStyle = lat === 0 ? 'rgba(0,229,255,0.18)' : 'rgba(0,229,255,0.06)';
    gctx.lineWidth = lat === 0 ? 0.7 : 0.4;
    gctx.beginPath();
    for (let lon = -180; lon <= 180; lon += 3) {
      const p = projectGlobe(lon, lat, globeAngle, cx, cy, gr);
      if (!p) continue;
      lon === -180 || !projectGlobe(lon - 3, lat, globeAngle, cx, cy, gr) ? gctx.moveTo(p.x, p.y) : gctx.lineTo(p.x, p.y);
    }
    gctx.stroke();
  }
  for (let lon = 0; lon < 360; lon += 20) {
    gctx.strokeStyle = 'rgba(0,229,255,0.06)';
    gctx.lineWidth = 0.4;
    gctx.beginPath();
    let first = true;
    for (let lat = -90; lat <= 90; lat += 2) {
      const p = projectGlobe(lon - 180, lat, globeAngle, cx, cy, gr);
      if (!p) { first = true; continue; }
      first ? gctx.moveTo(p.x, p.y) : gctx.lineTo(p.x, p.y);
      first = false;
    }
    gctx.stroke();
  }
  CONTINENTS.forEach(poly => {
    gctx.beginPath();
    let started = false;
    poly.forEach(([lon, lat]) => {
      const p = projectGlobe(lon, lat, globeAngle, cx, cy, gr);
      if (!p) { started = false; return; }
      if (!started) { gctx.moveTo(p.x, p.y); started = true; }
      else gctx.lineTo(p.x, p.y);
    });
    gctx.closePath();
    gctx.fillStyle = 'rgba(0,60,30,0.85)';
    gctx.fill();
    gctx.strokeStyle = 'rgba(0,229,255,0.35)';
    gctx.lineWidth = 0.7;
    gctx.stroke();
  });
  const posDot = projectGlobe(25, 35, globeAngle, cx, cy, gr);
  if (posDot) {
    gctx.fillStyle   = '#00E5FF';
    gctx.shadowColor = '#00E5FF';
    gctx.shadowBlur  = 8;
    gctx.beginPath();
    gctx.arc(posDot.x, posDot.y, 3, 0, Math.PI * 2);
    gctx.fill();
    gctx.globalAlpha = 0.3 + 0.2 * Math.sin(Date.now() * 0.005);
    gctx.beginPath();
    gctx.arc(posDot.x, posDot.y, 6 + 3 * Math.sin(Date.now() * 0.004), 0, Math.PI * 2);
    gctx.strokeStyle = '#00E5FF';
    gctx.lineWidth = 0.8;
    gctx.stroke();
    gctx.globalAlpha = 1;
    gctx.shadowBlur  = 0;
  }
  const specGrad = gctx.createRadialGradient(cx - 28, cy - 28, 0, cx, cy, gr);
  specGrad.addColorStop(0, 'rgba(255,255,255,0.07)');
  specGrad.addColorStop(0.5, 'transparent');
  gctx.fillStyle = specGrad;
  gctx.fillRect(cx - gr, cy - gr, gr * 2, gr * 2);
  gctx.restore();
  gctx.strokeStyle = 'rgba(0,180,255,0.45)';
  gctx.lineWidth = 1.2;
  gctx.beginPath();
  gctx.arc(cx, cy, gr, 0, Math.PI * 2);
  gctx.stroke();
  globeAngle += 0.003;
  requestAnimationFrame(drawGlobe);
}

// ── Perimeter ──────────────────────────────────────────────────
// FIX: perimeterPoints are stored in LOGICAL canvas coords (same space as CX, CY, R)
let perimeterPoints = [], isDrawingPerimeter = false;

function togglePerimeterDraw() {
  isDrawingPerimeter = !isDrawingPerimeter;
  const btn = document.getElementById('periDrawBtn');
  btn.classList.toggle('drawing', isDrawingPerimeter);
  btn.textContent = isDrawingPerimeter ? 'DRAWING…' : 'DRAW';
  if (isDrawingPerimeter) perimeterPoints = [];
  radarCanvas.style.cursor = isDrawingPerimeter ? 'crosshair' : (manualModeActive ? 'crosshair' : 'default');
}

function clearPerimeter() {
  perimeterPoints = []; isDrawingPerimeter = false;
  document.getElementById('periDrawBtn').classList.remove('drawing');
  document.getElementById('periDrawBtn').textContent = 'DRAW';
  radarCanvas.style.cursor = manualModeActive ? 'crosshair' : 'default';
}

function pointInPolygon(x, y, poly) {
  let inside = false;
  for (let i = 0, j = poly.length - 1; i < poly.length; j = i++) {
    const xi = poly[i].x, yi = poly[i].y, xj = poly[j].x, yj = poly[j].y;
    if (((yi > y) !== (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) inside = !inside;
  }
  return inside;
}

function pointInPerimeter(angle, dist) {
  if (perimeterPoints.length < 3 || dist <= 0) return false;
  const rad = servoToRad(angle), scale = Math.min(dist / MAX_RANGE_CM, 1);
  return pointInPolygon(CX + R * scale * Math.cos(rad), CY - R * scale * Math.sin(rad), perimeterPoints);
}

// FIX: Convert mouse event to logical canvas coords correctly.
// radarCanvas.getBoundingClientRect() gives CSS-pixel rect.
// radarCanvas.width/height are physical pixels.
// We need logical pixels = CSS pixels (since rctx is already scaled by DPR).
function canvasLogicalPos(e) {
  const rect = radarCanvas.getBoundingClientRect();
  const scaleX = (radarCanvas.width  / dpr) / rect.width;
  const scaleY = (radarCanvas.height / dpr) / rect.height;
  return {
    x: (e.clientX - rect.left) * scaleX,
    y: (e.clientY - rect.top)  * scaleY
  };
}

radarCanvas.addEventListener('click', (e) => {
  const pos = canvasLogicalPos(e);
  const lx = pos.x, ly = pos.y;

  if (isDrawingPerimeter) {
    perimeterPoints.push({ x: lx, y: ly });
    return;
  }

  if (manualModeActive) {
    // FIX: derive angle from logical coords relative to (CX, CY)
    const dx = lx - CX;
    const dy = CY - ly;  // invert Y so up = positive
    // Only accept clicks in the upper semicircle (dy >= 0 means above baseline)
    if (dy < 0) return;
    // atan2(dy, dx) gives angle from +X axis; we want servo angle
    // servoToRad(a) = (180-a)*PI/180  =>  a = 180 - atan2(dy,dx)*180/PI
    const angleDeg = Math.round(180 - Math.atan2(dy, dx) * 180 / Math.PI);
    const clamped  = Math.max(0, Math.min(180, angleDeg));
    currentAngle   = clamped;
    wsSend({ cmd: 'MANUAL_STEER', val: clamped });
  }
});

// Also handle mousemove for manual steer (live tracking while button held)
let mouseDown = false;
radarCanvas.addEventListener('mousedown', (e) => { mouseDown = true; });
radarCanvas.addEventListener('mouseup',   (e) => { mouseDown = false; });
radarCanvas.addEventListener('mousemove', (e) => {
  if (!manualModeActive || !mouseDown) return;
  const pos = canvasLogicalPos(e);
  const dx = pos.x - CX, dy = CY - pos.y;
  if (dy < 0) return;
  const angleDeg = Math.round(180 - Math.atan2(dy, dx) * 180 / Math.PI);
  const clamped  = Math.max(0, Math.min(180, angleDeg));
  currentAngle   = clamped;
  wsSend({ cmd: 'MANUAL_STEER', val: clamped });
});

// ── Doppler ────────────────────────────────────────────────────
const angleHistory = {};

function updateDoppler(angle, dist) {
  if (dist <= 0) return;
  const key = Math.round(angle / 5) * 5;
  if (!angleHistory[key]) angleHistory[key] = [dist];
  else { angleHistory[key].unshift(dist); if (angleHistory[key].length > 2) angleHistory[key].pop(); }
  if (angleHistory[key].length < 2) return;
  const delta = angleHistory[key][0] - angleHistory[key][1];
  const abs   = Math.abs(delta);
  const pct   = Math.min((abs / 20) * 100, 100);
  document.getElementById('motionBar').style.width      = pct + '%';
  document.getElementById('motionBar').style.background =
    delta < -1 ? 'var(--red)' : delta > 1 ? 'var(--green)' : 'var(--cyan)';
  document.getElementById('motionVal').textContent =
    abs < 1 ? 'STATIC' : delta < 0 ? `▼ ${abs.toFixed(1)}cm APPR` : `▲ ${abs.toFixed(1)}cm AWAY`;
}

// ── Alert log ──────────────────────────────────────────────────
let lastDetected = false;
let alertCount   = 0;
let unreadAlerts = 0;
const MAX_LOG = 100;

function logEvent(type, angle, dist) {
  if (type !== 'intercept') return;
  const now  = new Date();
  const ts   = now.toTimeString().slice(0, 8);
  const zone = dist <= INNER_ZONE_CM ? 'INNER' : 'OUTER';
  const div  = document.createElement('div');
  div.className = `log-entry ${dist <= INNER_ZONE_CM ? 'inner' : 'outer'}`;
  div.innerHTML = `<span class="log-ts">[${ts}]</span><span class="log-zone">${zone}</span>${angle}° · ${Math.round(dist)}cm`;
  const log = document.getElementById('alertLog');
  log.insertBefore(div, log.firstChild);
  while (log.children.length > MAX_LOG) log.removeChild(log.lastChild);
  alertCount++;
  document.getElementById('logCount').textContent = `${alertCount} event${alertCount !== 1 ? 's' : ''}`;
  if (!document.getElementById('tabLog').classList.contains('active')) {
    unreadAlerts++;
    const badge = document.getElementById('alertBadge');
    badge.textContent = unreadAlerts > 99 ? '99+' : unreadAlerts;
    badge.classList.add('visible');
  }
  pingAudio(dist);
}

function clearLog() {
  document.getElementById('alertLog').innerHTML = '';
  alertCount = 0;
  document.getElementById('logCount').textContent = '0 events';
}

function switchTab(name) {
  ['live', 'log'].forEach(t => {
    document.getElementById(`tab${t.charAt(0).toUpperCase()+t.slice(1)}`).classList.toggle('active', t === name);
    document.getElementById(`content${t.charAt(0).toUpperCase()+t.slice(1)}`).classList.toggle('active', t === name);
  });
  if (name === 'log') {
    unreadAlerts = 0;
    const badge = document.getElementById('alertBadge');
    badge.textContent = '0';
    badge.classList.remove('visible');
  }
}

// ── Audio ──────────────────────────────────────────────────────
let audioCtx = null, audioEnabled = false;
function getAudioCtx() {
  if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
  return audioCtx;
}
function pingAudio(dist) {
  if (!audioEnabled) return;
  const ctx  = getAudioCtx();
  const osc  = ctx.createOscillator();
  const gain = ctx.createGain();
  osc.type = 'sine';
  osc.frequency.value = 200 + (1 - Math.min(dist / MAX_RANGE_CM, 1)) * 1000;
  gain.gain.setValueAtTime(0.5, ctx.currentTime);
  gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.35);
  osc.connect(gain); gain.connect(ctx.destination);
  osc.start(); osc.stop(ctx.currentTime + 0.35);
}
function toggleAudio() {
  audioEnabled = !audioEnabled;
  document.getElementById('audioToggle').classList.toggle('on', audioEnabled);
  if (audioEnabled) getAudioCtx().resume();
}

// ── Manual mode ────────────────────────────────────────────────
let manualModeActive = false;

function toggleManual() {
  manualModeActive = !manualModeActive;
  document.getElementById('manualToggle').classList.toggle('active', manualModeActive);
  document.getElementById('manualBadge').classList.toggle('visible', manualModeActive);
  document.getElementById('sMode').textContent = manualModeActive ? 'MANUAL' : 'AUTO';
  document.getElementById('sMode').className   = 's-val ' + (manualModeActive ? 'warn' : '');
  radarCanvas.style.cursor = manualModeActive ? 'crosshair' : 'default';
  wsSend({ cmd: manualModeActive ? 'MANUAL_STEER' : 'AUTO_RESUME', val: currentAngle });
}

// ── Slider handlers ────────────────────────────────────────────
function onRadarRange(v) {
  v = parseInt(v);
  MAX_RANGE_CM = v;
  document.getElementById('valRadarRange').textContent    = v + 'cm';
  document.getElementById('radarRangeSummary').textContent = v + 'cm';
  wsSend({ cmd: 'SET_MAX_RANGE', val: v });
}
function onSweepMin(v) {
  v = parseInt(v); sweepMinDeg = v;
  document.getElementById('valSweepMin').textContent = v + '°';
  document.getElementById('sweepArcSummary').textContent = sweepMinDeg + '° – ' + sweepMaxDeg + '°';
  wsSend({ cmd: 'SET_SWEEP_MIN', val: v });
}
function onSweepMax(v) {
  v = parseInt(v); sweepMaxDeg = v;
  document.getElementById('valSweepMax').textContent = v + '°';
  document.getElementById('sweepArcSummary').textContent = sweepMinDeg + '° – ' + sweepMaxDeg + '°';
  wsSend({ cmd: 'SET_SWEEP_MAX', val: v });
}
let _stepDelay = 25, _stepDeg = 2, _threshCm = 40;
function _updateSensorSummary() {
  document.getElementById('sensorSummary').textContent = `${_stepDelay}ms · ${_stepDeg}° · ${_threshCm}cm`;
}
function onStepDelay(v) {
  v = parseInt(v); _stepDelay = v;
  document.getElementById('valDelay').textContent = v + 'ms';
  _updateSensorSummary();
  wsSend({ cmd: 'SET_STEP_DELAY', val: v });
}
function onStepDeg(v) {
  v = parseInt(v); _stepDeg = v;
  document.getElementById('valStep').textContent = v + '°';
  _updateSensorSummary();
  wsSend({ cmd: 'SET_STEP_DEG', val: v });
}
function onThreshold(v) {
  v = parseInt(v); thresholdCm = v; _threshCm = v;
  document.getElementById('valThreshold').textContent = v + 'cm';
  _updateSensorSummary();
  wsSend({ cmd: 'SET_THRESHOLD', val: v });
}

// ── Incoming frame ─────────────────────────────────────────────
function processFrame(data) {
  const angle = data.a, dist = data.d, det = data.det;
  currentAngle = angle; currentDist = dist; currentDet = det;

  const inInner  = dist > 0 && dist <= INNER_ZONE_CM;
  const periAlert = pointInPerimeter(angle, dist);

  document.getElementById('hdrAngle').textContent = `${angle}°`;
  document.getElementById('hdrDist').textContent  = dist < 0 ? 'OOR' : `${Math.round(dist)}cm`;

  document.getElementById('coordAngle').innerHTML = `${angle}<span class="coord-unit">°</span>`;
  document.getElementById('coordDist').innerHTML  = dist < 0
    ? `<span style="font-size:12px;color:var(--text-dim)">OOR</span>`
    : `${Math.round(dist)}<span class="coord-unit">cm</span>`;

  document.getElementById('sZone').textContent = dist <= 0 ? '--' : inInner ? 'INNER' : 'OUTER';
  document.getElementById('sZone').className   = 's-val ' + (inInner ? 'offline' : '');

  const banner = document.getElementById('alertBanner');
  if (periAlert) {
    banner.className = 'alert-banner breach'; banner.textContent = '⚠ PERIMETER BREACH';
  } else if (det) {
    banner.className = 'alert-banner threat'; banner.textContent = inInner ? '⚠ INNER ZONE THREAT' : '⚠ TARGET DETECTED';
  } else {
    banner.className = 'alert-banner clear';  banner.textContent = 'AIRSPACE CLEAR';
  }

  const lock = document.getElementById('lockStatus');
  if (det && inInner)  { lock.className = 'lock-status locked';   lock.textContent = 'LOCK — INNER THREAT'; }
  else if (det)        { lock.className = 'lock-status tracking'; lock.textContent = 'TRACKING — OUTER ZONE'; }
  else                 { lock.className = 'lock-status clear';    lock.textContent = 'NO TARGET'; }

  if (det && !lastDetected) logEvent('intercept', angle, dist);
  lastDetected = det;

  updateDoppler(angle, dist);
  _pushTrailInner(angle, dist, det);
  waterfallPush(angle, dist, det);
  pushSignal(dist);
}

// ── Telemetry ──────────────────────────────────────────────────
let scanCount = 0;
let startTime = Date.now();

function updateTelemetry() {
  const elapsed = Math.floor((Date.now() - startTime) / 1000);
  const hh = String(Math.floor(elapsed / 3600)).padStart(2,'0');
  const mm = String(Math.floor((elapsed % 3600) / 60)).padStart(2,'0');
  const ss = String(elapsed % 60).padStart(2,'0');
  document.getElementById('telemUptime').textContent = `${hh}:${mm}:${ss}`;
  const cpu = 8  + Math.round(Math.random() * 18);
  const mem = 30 + Math.round(Math.random() * 12);
  document.getElementById('cpuBar').style.width = cpu + '%';
  document.getElementById('cpuPct').textContent = cpu + '%';
  document.getElementById('memBar').style.width = mem + '%';
  document.getElementById('memPct').textContent = mem + '%';
}
setInterval(updateTelemetry, 2000);
updateTelemetry();

// ── Render loop ────────────────────────────────────────────────
function renderLoop() {
  drawRadar();
  requestAnimationFrame(renderLoop);
}

// ── Init ───────────────────────────────────────────────────────
function init() {
  resizeRadar();
  resizeWaterfall();
  renderLoop();
  drawGlobe();
  wsConnect();
}

window.addEventListener('resize', () => { resizeRadar(); resizeWaterfall(); });
init();
</script>
</body>
</html>
)HTMLEOF";
