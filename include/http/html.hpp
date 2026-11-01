#pragma once

#include <Arduino.h>

// Single-page dashboard, fully self-contained so it works offline (http://f30.local or the fallback AP at 192.168.4.1).
const char html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<meta name="theme-color" content="#0e1117">
<title>BMW F30 Cluster Workbench</title>
<style>
:root {
  color-scheme: dark;
  --bg: #0e1117;
  --card: #161a22;
  --field: #11141a;
  --control: #1c212b;
  --control-hover: #232936;
  --border: #232936;
  --border-strong: #2f3645;
  --text: #e6e9ef;
  --text-2: #9aa3b2;
  --text-3: #646d7c;
  --lamp-off: #3a4150;
  --amber: #ff7500;
  --blue: #1b64ff;
  --green: #22c55e;
  --red: #ef4444;
  --font: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  --mono: ui-monospace, "SF Mono", "Cascadia Mono", "Roboto Mono", Menlo, Consolas, monospace;
}
* { box-sizing: border-box; }
html, body { margin: 0; }
body { min-height: 100vh; background: var(--bg); color: var(--text); font: 14px/1.45 var(--font); font-variant-numeric: tabular-nums; -webkit-font-smoothing: antialiased; -webkit-tap-highlight-color: transparent; }
button, input { font: inherit; color: inherit; }
button { -webkit-user-select: none; user-select: none; }
:focus-visible { outline: 2px solid var(--blue); outline-offset: 2px; }
[hidden] { display: none !important; }
.mono { font-family: var(--mono); }

/* Header */
.bar { background: var(--card); border-bottom: 1px solid var(--border); }
.bar-inner { max-width: 1280px; margin: 0 auto; padding: 12px 16px; display: flex; align-items: center; flex-wrap: wrap; gap: 10px 16px; }
.title { min-width: 0; margin-right: auto; }
.title h1 { margin: 0; font-size: 16px; font-weight: 600; letter-spacing: .01em; }
.title p { margin: 2px 0 0; font: 12px var(--mono); color: var(--text-3); }
.badges { display: flex; flex-wrap: wrap; gap: 8px; }
.badge { display: inline-flex; align-items: center; gap: 8px; height: 28px; padding: 0 10px; border: 1px solid var(--border-strong); border-radius: 6px; background: var(--field); font-size: 12.5px; color: var(--text-2); white-space: nowrap; }
.badge i { width: 8px; height: 8px; border-radius: 50%; background: var(--lamp-off); }
.badge[data-s=ok] { color: var(--text); }
.badge[data-s=ok] i { background: var(--green); }
.badge[data-s=warn] i { background: var(--amber); }
.badge[data-s=bad] i { background: var(--red); }
.badge[data-s=pending] i { background: var(--amber); animation: pulse 1s steps(1) infinite; }
@keyframes pulse { 50% { opacity: .25; } }

/* Layout */
.grid { max-width: 1280px; margin: 0 auto; padding: 16px; display: grid; gap: 14px; grid-template-columns: repeat(12, minmax(0, 1fr)); }
.c-speed, .c-rpm { grid-column: span 6; }
.c-ign, .c-mode, .c-bl { grid-column: span 4; }
.c-lights { grid-column: span 7; }
.c-ind { grid-column: span 5; }
.c-tel { grid-column: 1 / -1; }
.card { min-width: 0; padding: 14px 16px 16px; background: var(--card); border: 1px solid var(--border); border-radius: 8px; }
.card-head { display: flex; align-items: baseline; justify-content: space-between; flex-wrap: wrap; gap: 4px 10px; margin-bottom: 12px; }
.card-head h2 { margin: 0; font-size: 13px; font-weight: 600; }
.meta { font: 11.5px var(--mono); color: var(--text-3); white-space: nowrap; }
.meta.warn { color: var(--amber); }
.info { margin-top: 10px; font: 12px var(--mono); color: var(--text-2); }
.foot { display: flex; justify-content: space-between; flex-wrap: wrap; gap: 4px 12px; margin-top: 12px; padding-top: 10px; border-top: 1px solid var(--border); font: 11.5px var(--mono); color: var(--text-3); }

/* Controls */
.btn { height: 36px; padding: 0 12px; border: 1px solid var(--border-strong); border-radius: 6px; background: var(--control); cursor: pointer; }
.btn:hover { background: var(--control-hover); }
.btn:active { background: var(--field); }
.btn-sm { height: 28px; padding: 0 10px; font-size: 12px; }
.btn[aria-pressed=true] { border-color: var(--amber); color: var(--amber); }
.seg { display: grid; grid-auto-flow: column; grid-auto-columns: minmax(0, 1fr); border: 1px solid var(--border-strong); border-radius: 6px; background: var(--field); overflow: hidden; }
.seg button { height: 46px; padding: 0 6px; border: 0; border-left: 1px solid var(--border-strong); background: transparent; color: var(--text-2); font-size: 13.5px; font-weight: 600; cursor: pointer; }
.seg button:first-child { border-left: 0; }
.seg button:hover { background: var(--control); color: var(--text); }
.seg button[aria-checked=true] { background: var(--on-bg); color: var(--on-fg); }
.presets { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 6px; margin-top: 10px; }
.presets .btn { height: 44px; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: 1px; padding: 0 4px; line-height: 1.1; }
.presets b { font-size: 15px; font-weight: 600; }
.presets small { font-size: 11px; color: var(--text-3); }
.presets .btn[aria-current=true] { border-color: var(--amber); }
input[type=range] { -webkit-appearance: none; appearance: none; display: block; width: 100%; height: 28px; margin: 6px 0 0; background: transparent; cursor: pointer; --fill: 0%; }
input[type=range]::-webkit-slider-runnable-track { height: 6px; border-radius: 3px; background: linear-gradient(to right, var(--amber) var(--fill), #2a303c var(--fill)); }
input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 18px; height: 18px; margin-top: -6px; border-radius: 50%; background: var(--text); border: 2px solid var(--bg); }
input[type=range]::-moz-range-track { height: 6px; border-radius: 3px; background: #2a303c; }
input[type=range]::-moz-range-progress { height: 6px; border-radius: 3px; background: var(--amber); }
input[type=range]::-moz-range-thumb { width: 14px; height: 14px; border-radius: 50%; background: var(--text); border: 2px solid var(--bg); }
input[type=range]:disabled { opacity: .4; cursor: not-allowed; }
.unavailable { opacity: .45; cursor: not-allowed !important; }

/* Dials */
.dial { display: block; width: 100%; max-width: 330px; margin: 0 auto; }
.d-tick { stroke: #4a5262; stroke-width: 1.5; }
.d-tick.major { stroke: #c4cad4; stroke-width: 2.5; }
.d-label { fill: #aeb5c1; font: 500 11px var(--font); text-anchor: middle; dominant-baseline: central; }
.d-red { fill: none; stroke: var(--red); stroke-width: 4; }
.d-ring { fill: none; stroke: var(--border); stroke-width: 1; }
.d-needle { stroke: var(--amber); stroke-width: 4; stroke-linecap: round; }
.d-value { fill: var(--text); font: 600 42px var(--font); text-anchor: middle; font-variant-numeric: tabular-nums; }
.d-unit { fill: var(--text-3); font: 500 12px var(--font); text-anchor: middle; }
.d-hit { fill: none; stroke: transparent; stroke-width: 52; pointer-events: stroke; cursor: pointer; touch-action: none; }
.instrument .body { transition: opacity .2s; }
body[data-ign=off] .instrument .body { opacity: .4; }
.ign-flag { display: none; margin-left: 8px; font: 11px var(--mono); color: var(--text-3); }
body[data-ign=off] .ign-flag { display: inline; }

/* Illumination: status line sits above the slider */
.c-bl .info { margin: 0 0 4px; }

/* Tell-tales */
.tiles { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 8px; }
.tiles.three { grid-template-columns: repeat(3, minmax(0, 1fr)); }
.tile { --c: var(--green); display: flex; flex-direction: column; align-items: center; gap: 8px; min-width: 0; padding: 14px 6px 12px; border: 1px solid var(--border-strong); border-radius: 6px; background: var(--field); color: var(--lamp-off); cursor: pointer; }
.tile:hover { background: var(--control); }
.tile svg { width: 36px; height: 36px; }
.tile .name { font-size: 12.5px; font-weight: 500; color: var(--text-2); text-align: center; }
.tile .st { font: 11px var(--mono); color: var(--text-3); }
.tile[aria-pressed=true] { color: var(--c); border-color: color-mix(in srgb, var(--c) 55%, var(--border-strong)); background: color-mix(in srgb, var(--c) 10%, var(--field)); }
.tile[aria-pressed=true] .st { color: var(--c); }
.tile.hazard { --c: var(--red); color: #6b2e33; }
.blink-off .tile.flash[aria-pressed=true] svg { color: var(--lamp-off); }

/* Telemetry drawer */
.c-tel { padding: 0; }
.drawer-toggle { display: flex; align-items: center; flex-wrap: wrap; gap: 6px 10px; width: 100%; padding: 14px 16px; border: 0; background: transparent; text-align: left; cursor: pointer; }
.drawer-title { font-size: 13px; font-weight: 600; }
.drawer-sum { margin-left: auto; font: 11.5px var(--mono); color: var(--text-3); }
.chev { width: 14px; height: 14px; color: var(--text-3); transition: transform .2s; }
.drawer-toggle[aria-expanded=true] .chev { transform: rotate(90deg); }
.tel-body { padding: 0 16px 16px; border-top: 1px solid var(--border); }
.stats { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 1px; margin: 14px 0; border: 1px solid var(--border); border-radius: 6px; background: var(--border); overflow: hidden; }
.stats div { min-width: 0; padding: 10px 12px; background: var(--field); }
.stats dt { font-size: 11.5px; color: var(--text-3); }
.stats dd { margin: 3px 0 0; font: 13px var(--mono); overflow-wrap: anywhere; }
.mon { display: grid; grid-template-columns: minmax(0, 1fr) minmax(0, 1fr); gap: 14px; }
.mon-head { display: flex; align-items: center; gap: 10px; margin-bottom: 8px; }
.mon-head h3 { margin: 0 auto 0 0; font-size: 13px; font-weight: 600; }
.note { margin: 0 0 8px; padding: 8px 10px; border: 1px solid var(--border-strong); border-left: 3px solid var(--amber); border-radius: 4px; font-size: 12.5px; color: var(--text-2); }
.table-wrap { max-height: 340px; overflow: auto; border: 1px solid var(--border); border-radius: 6px; background: var(--field); }
table { width: 100%; border-collapse: collapse; font: 12.5px/1.3 var(--mono); }
th { position: sticky; top: 0; padding: 8px 10px; border-bottom: 1px solid var(--border); background: var(--card); font: 600 11.5px var(--font); color: var(--text-3); text-align: left; white-space: nowrap; }
td { padding: 6px 10px; border-bottom: 1px solid #1a1f28; white-space: nowrap; }
tr:last-child td { border-bottom: 0; }
td.id { color: var(--amber); }
td.name { font-family: var(--font); color: var(--text-2); }
td.dim, .flag { color: var(--text-3); }
td.chg { animation: chg 1.2s ease-out; }
@keyframes chg { from { background: color-mix(in srgb, var(--amber) 22%, transparent); color: var(--amber); } }
.flag { margin-right: 6px; font-size: 11px; }
.empty { padding: 28px 16px; text-align: center; font-size: 12.5px; color: var(--text-3); }

.toasts { position: fixed; left: 50%; bottom: 16px; z-index: 10; display: flex; flex-direction: column; gap: 8px; width: min(440px, calc(100vw - 32px)); transform: translateX(-50%); pointer-events: none; }
.toast { padding: 10px 12px; border: 1px solid var(--border-strong); border-left: 3px solid var(--red); border-radius: 6px; background: var(--control); font-size: 13px; }

@media (max-width: 1000px) {
  .c-ign, .c-mode { grid-column: span 6; }
  .c-bl, .c-lights, .c-ind { grid-column: 1 / -1; }
  .mon { grid-template-columns: minmax(0, 1fr); }
}
@media (max-width: 640px) {
  .grid { grid-template-columns: minmax(0, 1fr); gap: 12px; padding: 12px; }
  .grid > * { grid-column: 1 / -1 !important; }
  .card { padding: 12px 12px 14px; }
  .c-tel { padding: 0; }
  .bar-inner { padding: 10px 12px; }
  .tiles { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .stats { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .drawer-sum { width: 100%; margin-left: 24px; }
  .tel-body { padding: 0 12px 12px; }
}
@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after { animation: none !important; transition: none !important; }
}
</style>
</head>
<body data-ign="">
<header class="bar">
  <div class="bar-inner">
    <div class="title">
      <h1>BMW F30 Cluster Workbench</h1>
      <p id="host">ESP32-S3 · MCP2515</p>
    </div>
    <div class="badges">
      <span class="badge" id="wsBadge" data-s="pending" title="WebSocket link to the ESP32"><i></i><span id="wsText">Connecting</span></span>
      <span class="badge" id="canBadge" data-s="" title="CAN controller state reported by the ESP32"><i></i><span id="canText">MCP2515 unknown</span></span>
    </div>
  </div>
</header>

<main class="grid">
  <section class="card instrument c-speed">
    <div class="card-head"><h2>Speed<span class="ign-flag">ignition off</span></h2><span class="meta" data-tag="speed-value">--</span></div>
    <div class="body">
      <svg class="dial" id="speedDial" viewBox="0 0 300 224" aria-hidden="true"></svg>
      <input type="range" id="speed" data-sig="speed-value" min="0" max="260" step="1" value="0" aria-label="Speed in km/h">
      <div class="presets" id="speedPresets"></div>
    </div>
    <div class="foot"><span id="speedRaw">raw --</span></div>
  </section>

  <section class="card instrument c-rpm">
    <div class="card-head"><h2>RPM<span class="ign-flag">ignition off</span></h2><span class="meta" data-tag="rpm-value">--</span></div>
    <div class="body">
      <svg class="dial" id="rpmDial" viewBox="0 0 300 224" aria-hidden="true"></svg>
      <input type="range" id="rpm" data-sig="rpm-value" min="0" max="8000" step="50" value="0" aria-label="Engine speed in rpm">
      <div class="presets" id="rpmPresets"></div>
    </div>
    <div class="foot"><span id="rpmRaw">raw --</span><span>red zone 6500 rpm</span></div>
  </section>

  <section class="card c-ign">
    <div class="card-head"><h2>Ignition</h2><span class="meta" data-tag="ignition-state">--</span></div>
    <div class="seg" id="ignSeg" role="radiogroup" aria-label="Ignition"></div>
    <div class="info" id="ignInfo">--</div>
  </section>

  <section class="card c-mode">
    <div class="card-head"><h2>Drive mode</h2><span class="meta" data-tag="drive-mode-state">--</span></div>
    <div class="seg" id="modeSeg" role="radiogroup" aria-label="Drive mode"></div>
    <div class="info" id="modeInfo">--</div>
  </section>

  <section class="card c-bl">
    <div class="card-head"><h2>Cluster illumination</h2><span class="meta" data-tag="backlight-brightness">--</span></div>
    <div class="info" id="blInfo">--</div>
    <input type="range" id="backlight" data-sig="backlight-brightness" min="0" max="255" step="1" value="0" aria-label="Cluster illumination">
  </section>

  <section class="card c-lights">
    <div class="card-head"><h2>Lights</h2><span class="meta" data-tag="main-lights">--</span></div>
    <div class="tiles" id="lamps"></div>
    <div class="info" id="lampInfo">--</div>
  </section>

  <section class="card c-ind">
    <div class="card-head"><h2>Turn indicators</h2><span class="meta" data-tag="blinker-left">--</span></div>
    <div class="tiles three">
      <button class="tile flash" id="indL" data-sig="blinker-left" type="button" aria-pressed="false">
        <svg viewBox="0 0 24 24" aria-hidden="true"><path d="M2.5 12L10 4.5V9h11.5v6H10v4.5z" fill="currentColor"/></svg>
        <span class="name">Left</span><span class="st">OFF</span>
      </button>
      <button class="tile hazard flash" id="hazard" data-sig="blinker-left blinker-right" type="button" aria-pressed="false">
        <svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 3L22 20H2z" fill="none" stroke="currentColor" stroke-width="2" stroke-linejoin="round"/><path d="M12 9.6l4.4 7.2H7.6z" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"/></svg>
        <span class="name">Hazard</span><span class="st">OFF</span>
      </button>
      <button class="tile flash" id="indR" data-sig="blinker-right" type="button" aria-pressed="false">
        <svg viewBox="0 0 24 24" aria-hidden="true"><path d="M21.5 12L14 4.5V9H2.5v6H14v4.5z" fill="currentColor"/></svg>
        <span class="name">Right</span><span class="st">OFF</span>
      </button>
    </div>
    <div class="info" id="indInfo">--</div>
  </section>

  <section class="card c-tel">
    <button class="drawer-toggle" id="telToggle" type="button" aria-expanded="false" aria-controls="telBody">
      <svg class="chev" viewBox="0 0 16 16" aria-hidden="true"><path d="M6 3.5l4.5 4.5L6 12.5" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"/></svg>
      <span class="drawer-title">Telemetry &amp; CAN monitor</span>
      <span class="drawer-sum" id="telSum">--</span>
    </button>
    <div class="tel-body" id="telBody" hidden>
      <dl class="stats">
        <div><dt>Link</dt><dd id="stLink">--</dd></div>
        <div><dt>Frames received</dt><dd id="stRx">--</dd></div>
        <div><dt>Frames sent</dt><dd id="stTx">--</dd></div>
        <div><dt>Clients</dt><dd id="stClients">--</dd></div>
      </dl>
      <div class="mon">
        <div>
          <div class="mon-head"><h3>Transmitted</h3><span class="meta" id="txCount">--</span></div>
          <p class="note" id="txNote" hidden>CAN is offline. Payloads stay up to date, but nothing is sent until the MCP2515 responds.</p>
          <div class="table-wrap">
            <table><thead><tr><th>ID</th><th>Message</th><th>Period</th><th>Payload</th></tr></thead><tbody id="txBody"></tbody></table>
          </div>
        </div>
        <div>
          <div class="mon-head"><h3>Received</h3><span class="meta" id="rxCount">--</span><button class="btn btn-sm" id="pause" type="button" aria-pressed="false">Pause</button></div>
          <div class="table-wrap">
            <table><thead><tr><th>#</th><th>ID</th><th>Name</th><th>DLC</th><th>Data</th></tr></thead><tbody id="rxBody"></tbody></table>
            <div class="empty" id="rxEmpty" hidden></div>
          </div>
        </div>
      </div>
    </div>
  </section>
</main>
<div class="toasts" id="toasts" role="status" aria-live="polite"></div>

<script>
(() => {
'use strict';

const SPEED = 'speed-value';
const RPM = 'rpm-value';
const IGNITION = 'ignition-state';
const DRIVE = 'drive-mode-state';
const BACKLIGHT = 'backlight-brightness';
const BLINK_L = 'blinker-left';
const BLINK_R = 'blinker-right';

// 0x1A1 carries speed in 1/64 km/h steps. Change this if the needle disagrees with the slider.
const SPEED_RAW_PER_KMH = 64;
const SPEED_MAX = 260;
const RPM_MAX = 8000;
const RPM_RED = 6500;
const SPEED_PRESETS = [{ value: 0, caption: 'Stop' }, { value: 50, caption: 'City' }, { value: 90, caption: 'Road' }, { value: 140, caption: 'Highway' }];
const RPM_PRESETS = [{ value: 0, caption: 'Off' }, { value: 800, caption: 'Idle' }, { value: 3000, caption: 'Cruise' }, { value: 6500, caption: 'Redline' }];
const IGN_OFF = 0x08;
const IGN_ACC = 0x85;
const IGN_ON = 0x8A;
const IGNITION_STATES = [
  { value: IGN_OFF, label: 'OFF', bg: '#3a4150', fg: '#ffffff' },
  { value: IGN_ACC, label: 'ACC', bg: '#1b64ff', fg: '#ffffff' },
  { value: IGN_ON, label: 'ON', bg: '#22c55e', fg: '#06210f' },
];
const MODES = [
  { value: 2, label: 'Comfort', bg: '#3a4150', fg: '#ffffff' },
  { value: 4, label: 'Sport', bg: '#ff7500', fg: '#1f0e00' },
  { value: 5, label: 'Sport+', bg: '#ef4444', fg: '#ffffff' },
];
const LAMP_L = '<path d="M13 5.5c4.4 0 8 2.9 8 6.5s-3.6 6.5-8 6.5z" fill="currentColor"/>';
const LAMP_R = '<path d="M11 5.5c-4.4 0-8 2.9-8 6.5s3.6 6.5 8 6.5z" fill="currentColor"/>';
const beams = d => `<path d="${d}" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>`;
const wave = x => `<path d="M${x} 4.5q-1.6 1.9 0 3.8t0 3.8t0 3.8t0 3.8" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"/>`;
const LAMPS = [
  { sig: 'main-lights', label: 'Main lights', status: 'MAIN LIGHTS', color: 'var(--green)', icon: LAMP_L + beams('M10 7L3 9.2M10 10.3L3 12.5M10 13.7L3 15.9M10 17L3 19.2') },
  { sig: 'high-beam', label: 'High beam', status: 'HIGH BEAM', color: 'var(--blue)', icon: LAMP_L + beams('M3 7h7M3 10.3h7M3 13.7h7M3 17h7') },
  { sig: 'front-fog', label: 'Front fog', status: 'FRONT FOG', color: 'var(--green)', icon: LAMP_L + beams('M10 8L3 10M10 12L3 14M10 16L3 18') + wave(6.5) },
  { sig: 'rear-fog', label: 'Rear fog', status: 'REAR FOG', color: 'var(--amber)', icon: LAMP_R + beams('M14 8h7M14 12h7M14 16h7') + wave(17.5) },
];
const LABELS = { [SPEED]: 'Speed', [RPM]: 'RPM', [IGNITION]: 'Ignition', [DRIVE]: 'Drive mode', [BACKLIGHT]: 'Illumination', [BLINK_L]: 'Turn indicators', [BLINK_R]: 'Turn indicators' };
LAMPS.forEach(l => { LABELS[l.sig] = l.label; });
// From docs/F30-can-ids.md
const KNOWN_IDS = {
  0x0D7: 'Alive counter', 0x0F3: 'RPM', 0x12F: 'Ignition', 0x19B: 'Airbag', 0x1A1: 'Speed', 0x1F6: 'Blinkers',
  0x202: 'Backlight', 0x21A: 'Lights', 0x289: 'Cruise control', 0x297: 'Seatbelt', 0x2A7: 'Power steering',
  0x2C4: 'Engine temp', 0x33B: 'ACC', 0x349: 'Fuel', 0x369: 'TPMS', 0x36E: 'ABS', 0x36F: 'Parking brake',
  0x3A7: 'Drive mode', 0x3F9: 'Oil', 0x3FD: 'Gear', 0x5C0: 'Alerts',
};

const WS_SEND_INTERVAL = 40;
const HTTP_SEND_INTERVAL = 120;
const HOLD_MS = 450;
const STALE_MS = 4000;
const BLINK_MS = 350;

const $ = s => document.querySelector(s);
const $$ = s => Array.from(document.querySelectorAll(s));
const hex = (n, w = 2) => '0x' + (Math.max(0, Math.round(n)) >>> 0).toString(16).toUpperCase().padStart(w, '0');
const bytes = data => (data || []).map(b => (b & 0xFF).toString(16).toUpperCase().padStart(2, '0')).join(' ');
const esc = s => String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' })[c]);
const labelOf = name => LABELS[name] || name;
const reduceMotion = matchMedia('(prefers-reduced-motion: reduce)').matches;

const state = {
  values: {},
  meta: {},        // signal -> { factor, offset, bit_length, start_bit } from /api/scheduler
  info: {},        // signal -> { id, interval }
  available: new Set(),
  known: false,    // true once the ESP32 reported which signals exist
  schedule: [],
  tx: [],          // live payloads from the status message
  reported: {},    // signal values from the same status message
  frames: [],
  can: null,
  heap: null,
  uptime: null,
  clients: null,
  net: null,
  paused: false,
  telOpen: false,
};
const views = {};
const holdUntil = {};

//
// Signal encoding (mirrors DBCMessage::getBytes: raw = (value - offset) / factor, masked to bit_length)
//
function meta(name) { return state.meta[name] || { factor: 1, offset: 0, bit_length: 0 }; }
function toRaw(name, value) {
  const m = meta(name);
  let raw = Math.round((value - (m.offset || 0)) / (m.factor || 1));
  if (m.bit_length > 0 && m.bit_length < 53) {
    const span = 2 ** m.bit_length;
    raw = ((raw % span) + span) % span;
  }
  return raw;
}
function fromRaw(name, raw) {
  const m = meta(name);
  return raw * (m.factor || 1) + (m.offset || 0);
}
function isAvailable(name) { return !state.known || state.available.has(name); }
function rawOf(name, fallback) { return name in state.values ? toRaw(name, state.values[name]) : fallback; }
// "HAZARD · 0x30": the label plus the bits the signals occupy in their payload byte
function statusText(label, names) {
  let bits = 0;
  for (const name of names) {
    const m = state.meta[name];
    if (!m || m.start_bit == null) return label;
    bits |= rawOf(name, 0) << (m.start_bit % 8);
  }
  return `${label} · ${hex(bits)}`;
}

//
// Toasts
//
const recentToasts = new Map();
function toast(message) {
  const now = Date.now();
  if (now - (recentToasts.get(message) || 0) < 2500) return;
  recentToasts.set(message, now);
  const el = document.createElement('div');
  el.className = 'toast';
  el.textContent = message;
  $('#toasts').append(el);
  setTimeout(() => el.remove(), 3800);
}

//
// Applying values
//
function render(name, value) {
  if (views[name]) views[name](value);
}

function applySignal(name, value, fromServer) {
  if (typeof value !== 'number' || !isFinite(value)) return;
  if (fromServer && Date.now() < (holdUntil[name] || 0)) return;
  state.values[name] = value;
  render(name, value);
}

function setSignal(name, value) {
  if (!isAvailable(name)) {
    toast(`${labelOf(name)}: not in the active scheduler, so nothing transmits it.`);
    return false;
  }
  holdUntil[name] = Date.now() + HOLD_MS;
  state.values[name] = value;
  render(name, value);
  queueSend(name, value);
  return true;
}

function setFill(input) {
  input.style.setProperty('--fill', (input.value - input.min) / (input.max - input.min) * 100 + '%');
}

//
// Sending: changes made in the same tick go out as one batch, at most every 40 ms over WebSocket
//
const pending = new Map();
let sendTimer = 0;
let lastSend = 0;

function queueSend(name, value) {
  pending.set(name, value);
  if (sendTimer) return;
  const wait = Math.max(0, lastSend + (wsOpen() ? WS_SEND_INTERVAL : HTTP_SEND_INTERVAL) - performance.now());
  sendTimer = setTimeout(flushSend, wait);
}

function flushSend() {
  sendTimer = 0;
  lastSend = performance.now();
  const values = Object.fromEntries(pending);
  pending.clear();
  for (const name in values) holdUntil[name] = Math.max(holdUntil[name] || 0, Date.now() + HOLD_MS);
  if (wsOpen()) {
    ws.send(JSON.stringify({ type: 'set', values }));
    return;
  }
  for (const name in values) patchSignal(name, values[name]);
}

function patchSignal(name, value) {
  fetchTimeout('/api/signals/' + encodeURIComponent(name), { method: 'PATCH', body: new URLSearchParams({ value: String(value) }) })
    .then(async r => {
      if (r.ok) { lastHttpOk = Date.now(); renderStatus(); return; }
      let message = 'HTTP ' + r.status;
      try { message = (await r.json()).message || message; } catch (e) {}
      throw new Error(message);
    })
    .catch(e => toast(`${labelOf(name)}: ${e.message === 'Failed to fetch' || e.name === 'AbortError' ? 'ESP32 unreachable' : e.message}`));
}

function fetchTimeout(url, opts) {
  const ctl = new AbortController();
  const t = setTimeout(() => ctl.abort(), 3000);
  return fetch(url, Object.assign({ cache: 'no-store', signal: ctl.signal }, opts)).finally(() => clearTimeout(t));
}
async function getJSON(url) {
  const r = await fetchTimeout(url);
  if (!r.ok) throw new Error('HTTP ' + r.status);
  return r.json();
}

//
// WebSocket with auto-reconnect; HTTP polling keeps the page usable while it is down
//
let ws = null;
let retries = 0;
let reconnectAt = 0;
let reconnectTimer = 0;
let lastRx = 0;
let lastHttpOk = 0;
let pollTimer = 0;

function wsOpen() { return !!ws && ws.readyState === WebSocket.OPEN; }

function connect() {
  clearTimeout(reconnectTimer);
  reconnectAt = 0;
  if (ws && ws.readyState <= WebSocket.OPEN) return;
  let sock;
  try {
    sock = new WebSocket((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/ws');
  } catch (e) {
    startPolling();
    scheduleReconnect();
    return;
  }
  ws = sock;
  renderStatus();
  sock.onopen = () => {
    retries = 0;
    lastRx = Date.now();
    stopPolling();
    renderStatus();
  };
  sock.onmessage = e => {
    lastRx = Date.now();
    let msg;
    try { msg = JSON.parse(e.data); } catch (err) { return; }
    handleMessage(msg);
  };
  sock.onclose = () => { if (ws === sock) dropSocket(); };
  sock.onerror = () => { if (ws === sock) dropSocket(); };
}

function dropSocket() {
  const sock = ws;
  ws = null;
  if (sock) {
    sock.onclose = sock.onerror = sock.onmessage = null;
    try { sock.close(); } catch (e) {}
  }
  Object.assign(state, { can: null, heap: null, uptime: null, clients: null, net: null, tx: [] });
  startPolling();
  scheduleReconnect();
}

function scheduleReconnect() {
  clearTimeout(reconnectTimer);
  const delay = Math.min(8000, 500 * 2 ** retries++);
  reconnectAt = Date.now() + delay;
  reconnectTimer = setTimeout(connect, delay);
  renderStatus();
}

function startPolling() {
  if (pollTimer) return;
  pollTimer = setInterval(poll, 1500);
  poll();
}
function stopPolling() {
  clearInterval(pollTimer);
  pollTimer = 0;
}
async function poll() {
  try {
    const [sched, read] = await Promise.all([getJSON('/api/scheduler'), getJSON('/api/read')]);
    if (wsOpen()) return;
    lastHttpOk = Date.now();
    loadScheduler(sched);
    receiveFrames(read.frames || []);
  } catch (e) {}
  renderStatus();
}

function handleMessage(msg) {
  if (msg.type === 'state') {
    Object.assign(state, { can: !!msg.can, heap: msg.heap, uptime: msg.uptime, clients: msg.clients, net: msg.net || null, tx: msg.tx || [] });
    const signals = msg.signals || {};
    state.reported = signals;
    setAvailable(new Set(Object.keys(signals)));
    for (const name in signals) applySignal(name, signals[name], true);
    renderStatus();
    renderTx();
  } else if (msg.type === 'set') {
    for (const name in msg.values || {}) applySignal(name, msg.values[name], true);
  } else if (msg.type === 'frames') {
    receiveFrames(msg.frames || []);
  }
}

//
// Status: header badges and telemetry
//
function fmtUptime(ms) {
  if (ms == null) return '--';
  const s = Math.floor(ms / 1000);
  const h = Math.floor(s / 3600);
  const pad = n => String(n).padStart(2, '0');
  return h >= 24 ? `${Math.floor(h / 24)}d ${pad(h % 24)}h` : `${h}:${pad(Math.floor(s / 60) % 60)}:${pad(s % 60)}`;
}

function linkState() {
  if (wsOpen()) return { s: 'ok', text: 'Connected', detail: 'WebSocket' };
  if (Date.now() - lastHttpOk < 5000) return { s: 'warn', text: 'HTTP fallback', detail: 'HTTP polling' };
  if (ws && ws.readyState === WebSocket.CONNECTING) return { s: 'pending', text: 'Connecting', detail: 'WebSocket' };
  const wait = reconnectAt > Date.now() ? `retry in ${Math.ceil((reconnectAt - Date.now()) / 1000)} s` : 'retrying';
  return { s: 'bad', text: 'Offline', detail: wait };
}

function renderStatus() {
  const link = linkState();
  $('#wsBadge').dataset.s = link.s;
  $('#wsText').textContent = link.text;
  $('#wsBadge').title = 'WebSocket link: ' + link.detail;

  const can = state.can === null ? { s: '', text: 'MCP2515 unknown' } : state.can ? { s: 'ok', text: 'MCP2515 Active' } : { s: 'bad', text: 'MCP2515 Offline' };
  $('#canBadge').dataset.s = can.s;
  $('#canText').textContent = can.text;

  const heap = state.heap != null ? Math.round(state.heap / 1024) + ' kB' : '--';
  $('#telSum').textContent = state.can === null ? 'no live data'
    : `${state.can ? 'CAN ok' : 'no CAN'} · IP: ${state.net ? state.net.ip : '--'} · free heap: ${heap} · uptime: ${fmtUptime(state.uptime)}`;
  $('#stLink').textContent = link.s === 'ok' ? 'WebSocket' : link.s === 'bad' ? `Offline, ${link.detail}` : link.text;
  // Scheduled rate; Timer::run fires once more than `interval` ms have passed, hence the +1
  const perSec = state.schedule.reduce((n, t) => n + (t.messages || []).length * 1000 / (t.interval + 1), 0);
  $('#stTx').textContent = state.can ? `≈ ${Math.round(perSec)} /s` : state.can === false ? '0 /s' : '--';
  $('#stClients').textContent = state.clients != null ? state.clients : '--';
  $('#txNote').hidden = state.can !== false;
}

//
// Scheduler: which signals exist, their encoding, and which message/timer carries them
//
function loadScheduler(data) {
  const timers = (data && data.timers) || [];
  const available = new Set();
  state.schedule = timers;
  for (const timer of timers) {
    for (const msg of timer.messages || []) {
      for (const sig of msg.signals || []) {
        available.add(sig.name);
        state.meta[sig.name] = { factor: sig.factor, offset: sig.offset, bit_length: sig.bit_length, start_bit: sig.start_bit };
        state.info[sig.name] = { id: msg.id, interval: timer.interval };
      }
    }
  }
  setAvailable(available);
  renderTags();
  for (const timer of timers) for (const msg of timer.messages || []) for (const sig of msg.signals || []) applySignal(sig.name, sig.value, true);
  renderTx();
}

function setAvailable(set) {
  const same = state.known && set.size === state.available.size && [...set].every(n => state.available.has(n));
  if (same) return;
  state.known = true;
  state.available = set;
  for (const el of $$('[data-sig]')) {
    const sigs = el.dataset.sig.split(' ');
    const ok = sigs.every(isAvailable);
    el.classList.toggle('unavailable', !ok);
    if (el.tagName === 'INPUT') el.disabled = !ok;
    else el.setAttribute('aria-disabled', String(!ok));
  }
  for (const name of Object.keys(views)) {
    if (name in state.values || !isAvailable(name)) render(name, state.values[name] ?? 0);
  }
  renderTags();
}

function renderTags() {
  for (const el of $$('[data-tag]')) {
    const name = el.dataset.tag;
    const info = state.info[name];
    const missing = state.known && !state.available.has(name);
    el.classList.toggle('warn', missing);
    if (missing) el.textContent = 'not scheduled';
    else if (info) el.textContent = `${hex(info.id, 3)} · every ${info.interval} ms`;
  }
}

//
// Dials
//
const SVG_NS = 'http://www.w3.org/2000/svg';
const CX = 150;
const CY = 146;

function polar(r, deg) {
  const a = deg * Math.PI / 180;
  return [CX + r * Math.sin(a), CY - r * Math.cos(a)];
}
function arcPath(r, a0, a1) {
  const [x0, y0] = polar(r, a0);
  const [x1, y1] = polar(r, a1);
  return `M${x0.toFixed(2)} ${y0.toFixed(2)}A${r} ${r} 0 ${a1 - a0 > 180 ? 1 : 0} 1 ${x1.toFixed(2)} ${y1.toFixed(2)}`;
}

// 240 degree dial; the needle floats outside the centre so the digital readout stays readable
function makeDial(svg, { max, minor, major, label, redFrom, unit, onDrag }) {
  const add = (tag, attrs) => {
    const el = document.createElementNS(SVG_NS, tag);
    for (const k in attrs) el.setAttribute(k, attrs[k]);
    svg.append(el);
    return el;
  };
  const toDeg = v => -120 + Math.min(Math.max(v, 0), max) / max * 240;

  if (redFrom != null) add('path', { class: 'd-red', d: arcPath(123, toDeg(redFrom), 120) });
  for (let i = 0; i * minor <= max; i++) {
    const v = i * minor;
    const isMajor = v % major === 0;
    const deg = toDeg(v);
    const [x0, y0] = polar(isMajor ? 104 : 111, deg);
    const [x1, y1] = polar(118, deg);
    add('line', { class: 'd-tick' + (isMajor ? ' major' : ''), x1: x0.toFixed(2), y1: y0.toFixed(2), x2: x1.toFixed(2), y2: y1.toFixed(2) });
    if (isMajor) {
      const [tx, ty] = polar(134, deg);
      add('text', { class: 'd-label', x: tx.toFixed(2), y: ty.toFixed(2) }).textContent = label(v);
    }
  }
  add('circle', { class: 'd-ring', cx: CX, cy: CY, r: 60 });
  const valueText = add('text', { class: 'd-value', x: CX, y: CY + 12 });
  valueText.textContent = '0';
  add('text', { class: 'd-unit', x: CX, y: CY + 34 }).textContent = unit;
  const needle = add('line', { class: 'd-needle', x1: CX, y1: CY - 66, x2: CX, y2: CY - 112, transform: `rotate(-120 ${CX} ${CY})` });
  const hit = add('path', { class: 'd-hit', d: arcPath(100, -120, 120) });

  let shown = 0;
  let target = 0;
  let raf = 0;
  const step = () => {
    const diff = target - shown;
    shown = reduceMotion || Math.abs(diff) < max / 2000 ? target : shown + diff * 0.25;
    needle.setAttribute('transform', `rotate(${toDeg(shown).toFixed(2)} ${CX} ${CY})`);
    raf = shown === target ? 0 : requestAnimationFrame(step);
  };

  let dragging = false;
  const valueAt = e => {
    const p = new DOMPoint(e.clientX, e.clientY).matrixTransform(svg.getScreenCTM().inverse());
    const deg = Math.max(-120, Math.min(120, Math.atan2(p.x - CX, CY - p.y) * 180 / Math.PI));
    return (deg + 120) / 240 * max;
  };
  hit.addEventListener('pointerdown', e => { dragging = true; hit.setPointerCapture(e.pointerId); onDrag(valueAt(e)); });
  hit.addEventListener('pointermove', e => { if (dragging) onDrag(valueAt(e)); });
  hit.addEventListener('pointerup', () => { dragging = false; });
  hit.addEventListener('pointercancel', () => { dragging = false; });

  return {
    set(value, text) {
      target = Math.min(Math.max(value, 0), max);
      valueText.textContent = text;
      if (!raf) raf = requestAnimationFrame(step);
    },
  };
}

function buildPresets(container, presets, unit, onPick) {
  for (const p of presets) {
    const b = document.createElement('button');
    b.type = 'button';
    b.className = 'btn';
    b.dataset.value = p.value;
    b.setAttribute('aria-label', `${p.value} ${unit}, ${p.caption}`);
    b.innerHTML = `<b>${p.value}</b><small>${p.caption}</small>`;
    b.onclick = () => onPick(p.value);
    container.append(b);
  }
}
function markPreset(container, value) {
  for (const b of container.children) b.setAttribute('aria-current', String(+b.dataset.value === value));
}

// Speed
const speedInput = $('#speed');
const setSpeed = kmh => setSignal(SPEED, fromRaw(SPEED, Math.round(kmh * SPEED_RAW_PER_KMH)));
const speedDial = makeDial($('#speedDial'), { max: SPEED_MAX, minor: 10, major: 20, label: v => v, unit: 'km/h', onDrag: v => setSpeed(Math.round(v)) });
buildPresets($('#speedPresets'), SPEED_PRESETS, 'km/h', setSpeed);
speedInput.addEventListener('input', () => setSpeed(+speedInput.value));
views[SPEED] = v => {
  const raw = toRaw(SPEED, v);
  const kmh = raw / SPEED_RAW_PER_KMH;
  speedDial.set(kmh, String(Math.round(kmh)));
  speedInput.value = Math.min(kmh, SPEED_MAX);
  setFill(speedInput);
  markPreset($('#speedPresets'), Math.round(kmh));
  $('#speedRaw').textContent = 'raw ' + hex(raw, 5);
};

// Rev counter
const rpmInput = $('#rpm');
const setRpm = rpm => setSignal(RPM, Math.round(rpm / 50) * 50);
const rpmDial = makeDial($('#rpmDial'), { max: RPM_MAX, minor: 500, major: 1000, label: v => v / 1000, redFrom: RPM_RED, unit: 'rpm', onDrag: setRpm });
buildPresets($('#rpmPresets'), RPM_PRESETS, 'rpm', setRpm);
rpmInput.addEventListener('input', () => setRpm(+rpmInput.value));
views[RPM] = v => {
  const rpm = Math.max(0, Math.round(v));
  rpmDial.set(rpm, String(rpm));
  rpmInput.value = Math.min(rpm, RPM_MAX);
  setFill(rpmInput);
  markPreset($('#rpmPresets'), rpm);
  $('#rpmRaw').textContent = 'raw ' + hex(toRaw(RPM, v), 4);
};

//
// Segmented switches (ignition, drive mode)
//
function buildSeg(container, sig, options) {
  for (const o of options) {
    const b = document.createElement('button');
    b.type = 'button';
    b.setAttribute('role', 'radio');
    b.setAttribute('aria-checked', 'false');
    b.dataset.sig = sig;
    b.dataset.value = o.value;
    b.style.setProperty('--on-bg', o.bg);
    b.style.setProperty('--on-fg', o.fg);
    b.textContent = o.label;
    b.onclick = () => setSignal(sig, fromRaw(sig, o.value));
    container.append(b);
  }
  container.addEventListener('keydown', e => {
    const dir = { ArrowRight: 1, ArrowDown: 1, ArrowLeft: -1, ArrowUp: -1 }[e.key];
    if (!dir) return;
    e.preventDefault();
    const idx = options.findIndex(o => o.value === rawOf(sig, -1));
    const next = options[(idx + dir + options.length) % options.length];
    if (setSignal(sig, fromRaw(sig, next.value))) container.querySelector(`[data-value="${next.value}"]`).focus();
  });
}
function checkSeg(container, raw) {
  const any = [...container.children].some(b => +b.dataset.value === raw);
  for (const b of container.children) {
    const on = +b.dataset.value === raw;
    b.setAttribute('aria-checked', String(on));
    b.tabIndex = on || (!any && b === container.firstElementChild) ? 0 : -1;
  }
}

const ignSeg = $('#ignSeg');
buildSeg(ignSeg, IGNITION, IGNITION_STATES);
views[IGNITION] = v => {
  const raw = toRaw(IGNITION, v);
  const st = IGNITION_STATES.find(s => s.value === raw);
  checkSeg(ignSeg, raw);
  document.body.dataset.ign = raw === IGN_OFF ? 'off' : raw === IGN_ACC ? 'acc' : raw === IGN_ON ? 'on' : 'other';
  $('#ignInfo').textContent = `${st ? st.label : 'UNKNOWN'} · ${hex(raw)}`;
};

const modeSeg = $('#modeSeg');
buildSeg(modeSeg, DRIVE, MODES);
views[DRIVE] = v => {
  const raw = toRaw(DRIVE, v);
  const mode = MODES.find(m => m.value === raw);
  checkSeg(modeSeg, raw);
  $('#modeInfo').textContent = `${mode ? mode.label.toUpperCase() : 'UNKNOWN'} · ${hex(raw)}`;
};

//
// Cluster illumination
//
const blInput = $('#backlight');
blInput.addEventListener('input', () => setSignal(BACKLIGHT, fromRaw(BACKLIGHT, +blInput.value)));
views[BACKLIGHT] = v => {
  const raw = toRaw(BACKLIGHT, v);
  blInput.value = raw;
  setFill(blInput);
  $('#blInfo').textContent = `${Math.round(Math.min(raw, 255) / 255 * 100)}% · ${hex(raw)}`;
};

//
// Lights
//
for (const lamp of LAMPS) {
  const b = document.createElement('button');
  b.type = 'button';
  b.className = 'tile';
  b.dataset.sig = lamp.sig;
  b.style.setProperty('--c', lamp.color);
  b.setAttribute('aria-pressed', 'false');
  b.innerHTML = `<svg viewBox="0 0 24 24" aria-hidden="true">${lamp.icon}</svg><span class="name">${lamp.label}</span><span class="st">OFF</span>`;
  b.onclick = () => setSignal(lamp.sig, fromRaw(lamp.sig, (rawOf(lamp.sig, 0) & 1) ? 0 : 1));
  $('#lamps').append(b);
  views[lamp.sig] = v => {
    const ok = isAvailable(lamp.sig);
    const on = ok && (toRaw(lamp.sig, v) & 1) === 1;
    b.setAttribute('aria-pressed', String(on));
    b.querySelector('.st').textContent = ok ? (on ? 'ON' : 'OFF') : 'not scheduled';
    renderLampStatus();
  };
}
function renderLampStatus() {
  const lamps = LAMPS.filter(l => isAvailable(l.sig));
  const on = lamps.filter(l => (rawOf(l.sig, 0) & 1) === 1).map(l => l.status);
  $('#lampInfo').textContent = lamps.length ? statusText(on.join(' + ') || 'OFF', lamps.map(l => l.sig)) : 'not scheduled';
}

//
// Turn indicators: left and right cancel each other, hazard drives both. Sent as one batch.
//
const blinkOn = sig => (rawOf(sig, 0) & 1) === 1;
function setBlinkers(left, right) {
  if (!isAvailable(BLINK_L) || !isAvailable(BLINK_R)) {
    toast('Turn indicators: not in the active scheduler, so nothing transmits them.');
    return;
  }
  setSignal(BLINK_L, fromRaw(BLINK_L, left ? 1 : 0));
  setSignal(BLINK_R, fromRaw(BLINK_R, right ? 1 : 0));
}
$('#indL').onclick = () => setBlinkers(!(blinkOn(BLINK_L) && !blinkOn(BLINK_R)), false);
$('#indR').onclick = () => setBlinkers(false, !(blinkOn(BLINK_R) && !blinkOn(BLINK_L)));
$('#hazard').onclick = () => { const both = blinkOn(BLINK_L) && blinkOn(BLINK_R); setBlinkers(!both, !both); };
function renderIndicators() {
  const ok = isAvailable(BLINK_L) && isAvailable(BLINK_R);
  const l = ok && blinkOn(BLINK_L);
  const r = ok && blinkOn(BLINK_R);
  const tiles = [[$('#indL'), l], [$('#indR'), r], [$('#hazard'), l && r]];
  for (const [tile, on] of tiles) {
    tile.setAttribute('aria-pressed', String(on));
    tile.querySelector('.st').textContent = ok ? (on ? 'ON' : 'OFF') : 'not scheduled';
  }
  $('#indInfo').textContent = ok ? statusText(l && r ? 'HAZARD' : l ? 'LEFT' : r ? 'RIGHT' : 'OFF', [BLINK_L, BLINK_R]) : 'not scheduled';
}
views[BLINK_L] = renderIndicators;
views[BLINK_R] = renderIndicators;
// Shared 1.5 Hz phase (350 ms on, 350 ms off) so every lit indicator flashes together
if (!reduceMotion) setInterval(() => document.body.classList.toggle('blink-off'), BLINK_MS);

//
// Telemetry & CAN monitor
//
function idText(raw) {
  const ext = (raw & 0x80000000) !== 0;
  const id = raw & 0x1FFFFFFF;
  return { text: hex(id, ext ? 8 : 3) + (ext ? ' x' : ''), name: ext ? '' : (KNOWN_IDS[id] || '') };
}

// Rows are rebuilt only when the schedule changes. Payload cells are updated in place and flash when
// one of the message's signals changed, so the rolling counter alone does not trigger the highlight.
const txRows = new Map(); // message id -> { cell, signals, text, values }
let txLayout = '';

function flash(el) {
  el.classList.remove('chg');
  void el.offsetWidth; // restart the animation
  el.classList.add('chg');
}

function renderTx() {
  if (!state.telOpen) return;
  const layout = JSON.stringify(state.schedule.map(t => [t.interval, (t.messages || []).map(m => m.id)]));
  if (layout !== txLayout) {
    txLayout = layout;
    txRows.clear();
    const body = $('#txBody');
    body.innerHTML = '';
    for (const timer of state.schedule) {
      for (const msg of timer.messages || []) {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td class="id">${hex(msg.id, 3)}</td><td class="name">${esc(msg.name)}</td><td class="dim">${timer.interval} ms</td><td></td>`;
        txRows.set(msg.id, { cell: tr.lastElementChild, signals: (msg.signals || []).map(s => s.name), text: null, values: null });
        body.append(tr);
      }
    }
    if (!txRows.size) body.innerHTML = '<tr><td colspan="4" class="dim">Scheduler not loaded yet</td></tr>';
    $('#txCount').textContent = `${txRows.size} message${txRows.size === 1 ? '' : 's'}`;
  }

  const live = new Map((state.tx || []).map(t => [t.id, t]));
  for (const [id, row] of txRows) {
    const tx = live.get(id);
    const text = tx ? bytes(tx.data) : '';
    if (text === row.text) continue;
    row.cell.innerHTML = tx ? (tx.crc ? '<span class="flag" title="sendMessage() prepends a CRC-8 byte">CRC +</span>' : '') + text : '<span class="flag">needs WebSocket</span>';
    const values = row.signals.map(n => state.reported[n]).join();
    if (tx && row.text && values !== row.values) flash(row.cell);
    row.text = text;
    row.values = values;
  }
}

function receiveFrames(frames) {
  state.frames = frames;
  $('#stRx').textContent = `${frames.length} buffered`;
  if (!state.paused) renderRx();
}

function renderRx() {
  if (!state.telOpen) return;
  const frames = state.frames;
  let rows = '';
  for (let i = frames.length - 1; i >= 0; i--) {
    const f = frames[i];
    const id = idText(f.id);
    rows += `<tr><td class="dim">${frames.length - i}</td><td class="id">${id.text}</td><td class="name">${esc(id.name || '—')}</td><td>${f.dlc}</td><td>${bytes(f.data)}</td></tr>`;
  }
  $('#rxBody').innerHTML = rows;
  $('#rxCount').textContent = `${frames.length} buffered`;
  const empty = $('#rxEmpty');
  empty.hidden = frames.length > 0;
  empty.textContent = state.can === false
    ? 'CAN offline. The ESP32 retries the MCP2515 every 5 s.'
    : 'No frames yet. The MCP2515 does not receive its own transmissions; frames appear when the cluster talks on the bus.';
}

function setTelemetry(open) {
  state.telOpen = open;
  $('#telToggle').setAttribute('aria-expanded', String(open));
  $('#telBody').hidden = !open;
  try { localStorage.setItem('f30.telemetry', open ? '1' : '0'); } catch (e) {}
  if (open) { renderStatus(); renderTx(); renderRx(); }
}
$('#telToggle').onclick = () => setTelemetry(!state.telOpen);
$('#pause').onclick = e => {
  state.paused = !state.paused;
  e.currentTarget.setAttribute('aria-pressed', String(state.paused));
  e.currentTarget.textContent = state.paused ? 'Resume' : 'Pause';
  if (!state.paused) renderRx();
};

//
// Boot
//
function loadScheduleOnce() {
  getJSON('/api/scheduler').then(loadScheduler).catch(() => setTimeout(loadScheduleOnce, 3000));
}

$('#host').textContent = (location.host || 'offline preview') + ' · ESP32-S3 · MCP2515';
setFill(speedInput);
setFill(rpmInput);
setFill(blInput);
renderIndicators();
let telOpen = false;
try { telOpen = localStorage.getItem('f30.telemetry') === '1'; } catch (e) {}
setTelemetry(telOpen);
renderStatus();
loadScheduleOnce();
connect();

setInterval(() => {
  if (wsOpen() && Date.now() - lastRx > STALE_MS) dropSocket();
  renderStatus();
}, 1000);

document.addEventListener('visibilitychange', () => {
  if (document.visibilityState !== 'visible') return;
  if (wsOpen()) ws.send(JSON.stringify({ type: 'sync' }));
  else { retries = 0; connect(); }
});
})();
</script>
</body>
</html>
)rawliteral";
