#!/usr/bin/env python3
"""Local preview of the lamp's web pages with mock data, for design work in a browser on the PC.

Run from any directory:  py -3 Software/Web_Pages/preview_server.py
                         [--scenario normal|bad-air|sensor-fail|ap|sta-offline|errors|join-ok|join-wrong-pass|join-no-network|
                                     sta-switch-ok|sta-switch-wrong-pass|sta-switch-no-network|
                                     disp-effects|disp-errors|disp-night]
                         [--host 127.0.0.1] [--port 8080] [--latency MS] [--no-browser]

The page sources next to this script are read from disk on every request (Cache-Control: no-store), so saving a file
and refreshing the browser shows the change. The routes, JSON shapes, status codes, content types and reply texts of
HAQuDA_WebServer.cpp are reproduced for the success paths and for these error paths: the 400 and 409 replies of
/add_wifi_creds, the 500 replies of /api/status and /delete_wifi_creds (scenario "errors"), and the
index page for unknown GET URLs in the access point scenarios (the captive portal). The access point scenarios ("ap"
and join-*) answer like the AP-mode firmware: POST /add_wifi_creds answers 202, /api/join reports "checking" for about 4 s and then the result of the
scenario ("ap" passes like join-ok; join-wrong-pass and join-no-network fail); a network that passed is added to the
saved list.

The station scenarios (every other one) answer like the station-mode firmware: POST /add_wifi_creds answers 202 and the
lamp scans, announces the switch ("switching"), goes offline and comes back with the outcome in "last" of /api/join,
which stays until POST /api/join/dismiss or the next check. sta-switch-ok (and the plain scenarios) join the new network,
which becomes the first saved one; sta-switch-wrong-pass returns to the old network with "last" fail; sta-switch-no-network
fails at the scan at once and the lamp stays reachable. While offline the server closes every connection without an answer.

GET /api/disp answers with the picture the lamp shows, in the format of HAQuDA_TranslateDisplayToJson (the page grid is
4 columns x 6 rows). Scenario by scenario: the access point scenarios ("ap" and join-*) show the aqua fading effect the lamp
runs in access point mode, sta-offline the lime snake it runs while it reconnects, sensor-fail the random pixels of the sensors
error, "errors" the 500 reply of /api/status, every other scenario the multi layout (temperature, eCO2, PM2.5). Three scenarios
exist only to look at the pictures: disp-effects plays snake, growing, random, fading and static colour for 25 s each,
disp-errors plays the five error effects (WiFi, access point, file system, sensors, NTP) for 15 s each, disp-night shows the
night layout with the white part of the bar growing every 4 s. The measurement pictures carry "colors" (the scale colour of the
current mock values of their keys) and, for night, "whiteColor" (the grey the lamp draws at the mock brightness). In the cycling scenarios seq rises and age restarts at each
change; elsewhere seq stays 1 and age counts from the start of the server. The disp-* scenarios otherwise answer like a
station mode lamp.

Not reproduced: gzip, the ?v= asset
versions (the query is ignored), the restart after saving a network in the access point scenarios (the reply text is sent,
nothing restarts) and the WebSerial page. Received passwords are neither printed nor stored.
"""

import argparse
import json
import os
import random
import sys
import threading
import time
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# URL path -> (source file, content type); the types are the ones the firmware sends
ASSETS = {
    "/": ("index.html", "text/html"),
    "/wifi_manager": ("wifi_manager.html", "text/html"),
    "/style.css": ("style.css", "text/css"),
    "/app.js": ("app.js", "application/javascript"),
    "/wifi.js": ("wifi.js", "application/javascript"),
}

SCENARIOS = ["normal", "bad-air", "sensor-fail", "ap", "sta-offline", "errors", "join-ok", "join-wrong-pass", "join-no-network",
             "sta-switch-ok", "sta-switch-wrong-pass", "sta-switch-no-network", "disp-effects", "disp-errors", "disp-night"]
# Scenarios in which the lamp runs as an access point: a new network is checked before it is saved
AP_SCENARIOS = {"ap", "join-ok", "join-wrong-pass", "join-no-network"}
# How long the mock check runs before it reports its result (the lamp takes up to 20 s)
JOIN_CHECK_S = 4.0
# What the mock check reports in each scenario: None = the network works, else the reason
JOIN_FAIL_REASON = {"ap": None, "join-ok": None, "join-wrong-pass": "wrong_password", "join-no-network": "no_network"}
# The lamp holds access point and station together this long after a passed check, then restarts
JOIN_OK_HOLD_S = 20.0
# Network the mock lamp is joined to in station mode
STA_SSID = "HomeNet"
JOIN_HOST = "haquda.local"
# The address the mock check reports as the lamp's new one. It points back at this machine, so the page's automatic
# redirect can be tried: start the preview with --port 80 and the redirect after a passed check (about 22 s plus the
# probe round) lands on this server's own page; on any other port the probe of http://127.0.0.1/ finds nothing
JOIN_IP = "127.0.0.1"
JOIN_BUSY_TEXT = "The lamp is already checking a network. Wait for the result."
JOIN_RESTARTING_TEXT = "The network is saved. The lamp is restarting."
JOIN_ACCEPTED_TEXT = "The lamp is checking the network. Open this page again in 30 seconds."
STA_ACCEPTED_TEXT = (
    "The lamp is checking the network. This page may be unreachable for about a minute while the lamp tries it. "
    "Open this page again then to see the result."
)

# Station mode switch, in seconds from the POST: the scan, the announce (state "switching" while the lamp is still
# reachable), then the lamp is away this long (leave, join, restart, boot) and comes back with the outcome
STA_SCAN_S = 1.5
STA_ANNOUNCE_S = 2.5
STA_OFFLINE_S = 10.0
# What the mock switch reports: None = the network works, else the reason. "no_network" fails at the scan
# without the lamp leaving; any other reason is reported after the restart. Scenarios not listed pass.
STA_FAIL_REASON = {"sta-switch-wrong-pass": "wrong_password", "sta-switch-no-network": "no_network"}

# Reply of the lamp's 500 path
STATUS_FAIL_TEXT = "Status unavailable."
DELETE_FAIL_TEXT = "Error deleting the saved networks."

# Limits of the lamp's WiFi record (HAQuDA_FileStorage.h): a field holds the text and a terminator
MAX_WIFI_CREDS_NUM = 10
WIFI_SSID_LEN = 32
WIFI_PASS_LEN = 32
MAX_BODY_BYTES = 4096
FW_VERSION = "2.0.0"

WEBSERIAL_STUB = (
    '<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><title>WebSerial</title>'
    '<meta name="viewport" content="width=device-width, initial-scale=1"></head>'
    '<body style="font-family:sans-serif;margin:2em"><h1>WebSerial</h1>'
    "<p>WebSerial is available only on the lamp.</p>"
    '<p><a href="/">Back to the main page</a></p></body></html>'
)

# key, decimals shown, (blue-below, green-at, red-above) dots, calm value, bad-air value, random-walk step
# The dots are plausible stand-ins, not the lamp's configured divide dots.
PARAMS = [
    ("temp", 1, (18.0, 23.0, 28.0), 23.4, 29.0, 0.15),
    ("humid", 1, (30.0, 45.0, 60.0), 44.0, 65.0, 0.4),
    ("eco2", 0, (600.0, 1000.0, 1500.0), 720.0, 2300.0, 25.0),
    ("tvoc", 0, (50.0, 220.0, 660.0), 90.0, 950.0, 12.0),
    ("pm25", 0, (12.0, 35.0, 75.0), 9.0, 120.0, 2.0),
]
# Parameters the failed sensors would leave empty in the sensor-fail scenario (CCS811 and ZH03B)
FAILED_KEYS = {"eco2", "tvoc", "pm25"}


def scale_color(value, dots):
    """Simple stand-in for the firmware's colour scale: blue below first, green at mid, red above last.

    Linear blend between the anchors; the firmware's own scale (HAQuDA_ColorScale) is not ported.
    """
    first, mid, last = dots
    blue, green, red = (0, 0, 255), (0, 255, 0), (255, 0, 0)
    if value <= first:
        rgb = blue
    elif value >= last:
        rgb = red
    elif value <= mid:
        t = (value - first) / (mid - first)
        rgb = tuple(round(blue[i] + (green[i] - blue[i]) * t) for i in range(3))
    else:
        t = (value - mid) / (last - mid)
        rgb = tuple(round(green[i] + (red[i] - green[i]) * t) for i in range(3))
    return "#{0:02x}{1:02x}{2:02x}".format(*rgb)


# The lamp's picture for the page (HAQuDA_TranslateDisplayToJson): the lamp is 109 pixels in 12 rows, the page grid 24 cells in 6 rows
LAMP_PIXELS = 109
LAMP_ROWS = 12
PAGE_CELLS = 24
PAGE_ROWS = 6
SHOW_PAUSE_MS = 100
# HAQuDA_LEDs::Show() without an argument blocks 200 ms, so do Clear() and SetBright(); every effect starts with Clear()
LEDS_SHOW_MS = 200
CLEAR_DARK_MS = LEDS_SHOW_MS
FADING_DARK_MS = 2 * LEDS_SHOW_MS
DISP_BRIGHT = 50
DISP_EFFECT_SLOT_S = 25.0
DISP_ERROR_SLOT_S = 15.0
DISP_NIGHT_SLOT_S = 4.0
# The keys the lamp shows in its multi layout by default
DISP_MULTI_KEYS = ["temp", "eco2", "pm25"]


def div_round(numerator, denominator):
    return (numerator + denominator // 2) // denominator


def fx_snake(color, move_pause_ms, tail_len):
    lap_ms = LAMP_PIXELS * move_pause_ms + SHOW_PAUSE_MS
    tail = min(max(div_round(tail_len * PAGE_CELLS, LAMP_PIXELS), 1), PAGE_CELLS)
    return {"fx": "snake", "color": color, "dark": CLEAR_DARK_MS, "step": div_round(lap_ms, PAGE_CELLS), "tail": tail}


def fx_growing(color, grow_pause_ms):
    return {"fx": "growing", "color": color, "dark": CLEAR_DARK_MS, "step": (LAMP_ROWS // PAGE_ROWS) * grow_pause_ms,
            "end": SHOW_PAUSE_MS}


def fx_random(pause_ms, end_pause_ms):
    return {"fx": "random", "dark": CLEAR_DARK_MS, "step": div_round(LAMP_PIXELS * pause_ms, PAGE_CELLS),
            "end": end_pause_ms + SHOW_PAUSE_MS}


def fx_fading(color, fade_pause_ms, start_bright, stop_bright, step):
    """The lamp is dark for Clear() and SetBright(start), shows the start level for 200 ms, then every level (the first is the
    start level again) for 200 ms + the pause; the last level stays 100 ms longer.

    When the countdown does not run (start not above stop) the lamp flashes: one level, from = to = start, shown for 200 ms
    (cycle 700 ms). Start above stop with step 0 is steady, a static colour.
    """
    counts = start_bright > stop_bright
    if counts and step == 0:
        return fx_static(scale_percent(color, start_bright))
    steps = -(-(start_bright - stop_bright) // step) if counts else 1
    step_ms = LEDS_SHOW_MS + fade_pause_ms
    return {"fx": "fading", "color": color, "from": start_bright, "to": stop_bright if counts else start_bright, "steps": steps,
            "dark": FADING_DARK_MS, "first": LEDS_SHOW_MS + step_ms if counts else LEDS_SHOW_MS, "step": step_ms,
            "end": SHOW_PAUSE_MS}


def scale_percent(color, percent):
    """The colour with every channel scaled by percent (0..100), rounded half up like the firmware."""
    value = int(color[1:], 16)
    scaled = 0
    for shift in (16, 8, 0):
        scaled |= div_round(((value >> shift) & 0xFF) * percent, 100) << shift
    return "#{0:06x}".format(scaled)


def fx_static(color):
    return {"fx": "static", "color": color}


def night_white_color(bright_percent):
    """The grey of the night bar as the lamp draws it: raw brightness * 70 / 150 as a channel, scaled from 0..150 to 0..255."""
    raw_bright = bright_percent * 150 // 100
    channel = raw_bright * 1000 // 150 * 70 // 1000
    return "#{0:02x}{0:02x}{0:02x}".format(channel * 255 // 150)


def fx_meas(layout, keys, colors, white=None):
    """colors: the colours the lamp drew, one per key (multi) or one (standard, night); white: page rows of the night bar."""
    picture = {"fx": "meas", "layout": layout, "keys": keys, "colors": colors}
    if white is not None:
        picture["white"] = white
        picture["whiteColor"] = night_white_color(DISP_BRIGHT)
    return picture


# The effect the lamp runs in access point mode (aqua fading) and while it reconnects (lime snake), the defaults of the lamp
DISP_AP_PICTURE = fx_fading("#00ffff", 10, 100, 0, 5)
DISP_RECONNECT_PICTURE = fx_snake("#99ff33", 200, 5)
# One effect after the other in disp-effects: (effect name, picture)
DISP_EFFECTS = [
    ("snake", DISP_RECONNECT_PICTURE),
    ("growing", fx_growing("#00ffff", 200)),
    ("random", fx_random(200, 1000)),
    ("fading", DISP_AP_PICTURE),
    ("static", fx_static("#01ff00")),
]
# The five error effects in disp-errors: (error kind, picture). The NTP error starts at brightness 0 and never counts down, so the lamp stays dark.
DISP_ERRORS = [
    ("wifi", fx_growing("#ff0000", 100)),
    ("ap", fx_snake("#ff0000", 100, 1)),
    ("fs", fx_fading("#ff0000", 10, 100, 0, 5)),
    ("sensors", fx_random(50, 500)),
    ("ntp", fx_fading("#ffff00", 10, 0, 100, 10)),
]


class MockLamp:
    """In-memory state of the lamp: slowly varying readings and the saved WiFi list. Thread-safe."""

    def __init__(self, scenario):
        self.scenario = scenario
        self.lock = threading.Lock()
        self.started = time.monotonic()
        # The lamp has run for a while already, so the uptime text shows days and hours
        self.uptime_offset_s = 3 * 86400 + 5 * 3600 + 12 * 60
        bad = scenario == "bad-air"
        self.values = {key: (bad_value if bad else calm) for key, _, _, calm, bad_value, _ in PARAMS}
        self.networks = ["HomeNet", 'Cafe "Wi\\Fi"', "Café 咖啡 \U0001F4F6"]
        self.rssi = -58
        # The check of the last POST in a join-* scenario: (ssid, start time); None until one was started
        self.join = None
        self.join_saved = False
        # Station scenarios: the network the lamp is joined to, the switch of the last POST (ssid, start time, reason or
        # None) and the outcome that survives the "restart" ({"result", "reason", "ssid"}), until dismissed or replaced
        self.current_ssid = STA_SSID
        self.switch = None
        self.last = None
        self.restarted_at = None

    def place_first(self, ssid):
        """Saves a passed network like the firmware: first in the list, a stored one of the same name replaced, the last one
        dropped when more than 10 would be kept. The caller holds the lock."""
        self.networks = [ssid] + [name for name in self.networks if name != ssid][: MAX_WIFI_CREDS_NUM - 1]

    def sta_flow(self):
        return self.scenario not in AP_SCENARIOS

    def sta_phase(self):
        """Phase of the mock station switch: idle, scanning, scan_failed, announcing, offline. The caller holds the lock."""
        if self.switch is None:
            return "idle"
        elapsed = time.monotonic() - self.switch[1]
        if elapsed < STA_SCAN_S:
            return "scanning"
        if self.switch[2] == "no_network":
            return "scan_failed"
        if elapsed < STA_SCAN_S + STA_ANNOUNCE_S:
            return "announcing"
        if elapsed < STA_SCAN_S + STA_ANNOUNCE_S + STA_OFFLINE_S:
            return "offline"
        # The lamp is back: it has restarted with the outcome of the switch
        ssid, _, reason = self.switch
        if reason is None:
            self.last = {"result": "ok", "ssid": ssid}
            # The new network becomes the first saved one and the lamp is joined to it
            self.place_first(ssid)
            self.current_ssid = ssid
        else:
            self.last = {"result": "fail", "reason": reason, "ssid": ssid}
        self.switch = None
        self.restarted_at = time.monotonic()
        return "idle"

    def is_offline(self):
        """True while the mock lamp is away during a station switch: the server answers nothing."""
        with self.lock:
            return self.sta_flow() and self.sta_phase() == "offline"

    def dismiss_last(self):
        with self.lock:
            self.last = None

    def current_uptime_s(self):
        """Uptime of the mock lamp. The caller holds the lock."""
        uptime = self.uptime_offset_s + int(time.monotonic() - self.started)
        if self.restarted_at is not None:
            return int(time.monotonic() - self.restarted_at)
        if self.scenario in JOIN_FAIL_REASON and self.join is not None and self.join_state() == "ok":
            # After a passed check the mock lamp "restarts" once its hold is over: the uptime starts again at zero,
            # which is what the page waits for before it follows the redirect
            since_restart_s = time.monotonic() - self.join[1] - JOIN_CHECK_S - JOIN_OK_HOLD_S
            if since_restart_s >= 0:
                uptime = int(since_restart_s)
        return uptime

    def boot_json(self):
        """Body of /api/boot like the lamp builds it: the firmware version, the uptime and the host name, nothing else."""
        with self.lock:
            if self.sta_flow():
                self.sta_phase()
            return '{{"fw":"{0}","uptime":{1},"host":"{2}"}}'.format(FW_VERSION, self.current_uptime_s(), JOIN_HOST.split(".")[0])

    def status_json(self):
        """Returns (status code, body)."""
        with self.lock:
            if self.sta_flow():
                self.sta_phase()
            if self.scenario == "errors":
                return 500, STATUS_FAIL_TEXT
            station = self.scenario not in AP_SCENARIOS
            connected = self.scenario != "sta-offline"
            uptime = self.current_uptime_s()
            parts = []
            for key, decimals, dots, calm, bad_value, step in PARAMS:
                target = bad_value if self.scenario == "bad-air" else calm
                # Random walk pulled back to the scenario's level so the values stay plausible
                self.values[key] += random.gauss(0, step) + (target - self.values[key]) * 0.05
                if self.scenario == "sensor-fail" and key in FAILED_KEYS:
                    parts.append('"{0}":null'.format(key))
                    continue
                value = self.values[key]
                parts.append('"{0}":{{"v":{1:.{2}f},"c":"{3}"}}'.format(key, value, decimals, scale_color(value, dots)))
            if station and connected:
                self.rssi = max(-85, min(-40, self.rssi + random.choice((-1, 0, 0, 1))))
                rssi = str(self.rssi)
            else:
                # The lamp sends null in AP mode and while the station is not connected
                rssi = "null"
            if not station:
                ip = "192.168.4.1"
            else:
                # An unconnected station has no address yet
                ip = "192.168.1.50" if connected else "0.0.0.0"
            # The lamp sends the joined network's name in station mode and null otherwise
            ssid = json.dumps(self.current_ssid, ensure_ascii=False) if station and connected else "null"
            return 200, '{{"fw":"{0}","mode":"{1}","ssid":{2},"ip":"{3}","uptime":{4},"rssi":{5},"meas":{{{6}}}}}'.format(
                FW_VERSION, "sta" if station else "ap", ssid, ip, uptime, rssi, ",".join(parts)
            )

    def drawn_colors(self, keys):
        """The colours the mock lamp 'drew' for the keys: the scale colour of their current values, like /api/status gives them."""
        dots = {name: dot_set for name, _, dot_set, _, _, _ in PARAMS}
        with self.lock:
            return [scale_color(self.values[key], dots[key]) for key in keys]

    def disp_json(self):
        """Returns (status code, body): the picture the lamp shows, like /api/disp of the firmware."""
        if self.scenario == "errors":
            return 500, STATUS_FAIL_TEXT
        elapsed_s = time.monotonic() - self.started
        error = None
        if self.scenario == "disp-effects":
            seq, age_s = divmod(elapsed_s, DISP_EFFECT_SLOT_S)
            picture = DISP_EFFECTS[int(seq) % len(DISP_EFFECTS)][1]
            source = "effect"
        elif self.scenario == "disp-errors":
            seq, age_s = divmod(elapsed_s, DISP_ERROR_SLOT_S)
            error, picture = DISP_ERRORS[int(seq) % len(DISP_ERRORS)]
            source = "error"
        elif self.scenario == "disp-night":
            # The hour into the night window runs 0..12, 12 fills the bar
            seq, age_s = divmod(elapsed_s, DISP_NIGHT_SLOT_S)
            hour = int(seq) % 13
            picture = fx_meas("night", ["temp"], self.drawn_colors(["temp"]), (hour * PAGE_ROWS + 11) // 12)
            source = "meas"
        else:
            seq, age_s = 0, elapsed_s
            if self.scenario in AP_SCENARIOS:
                picture, source = DISP_AP_PICTURE, "effect"
            elif self.scenario == "sta-offline":
                picture, source = DISP_RECONNECT_PICTURE, "effect"
            elif self.scenario == "sensor-fail":
                error, picture = DISP_ERRORS[3]
                source = "error"
            else:
                picture, source = fx_meas("multi", DISP_MULTI_KEYS, self.drawn_colors(DISP_MULTI_KEYS)), "meas"
        body = {"seq": int(seq) + 1, "age": int(age_s * 1000), "src": source}
        if error is not None:
            body["err"] = error
        body["bright"] = DISP_BRIGHT
        body.update(picture)
        return 200, json.dumps(body, separators=(",", ":"))

    def wifi_json(self):
        with self.lock:
            if self.sta_flow():
                self.sta_phase()
            # Raw UTF-8 like the lamp; only quote, backslash and control characters are escaped
            return json.dumps(self.networks, ensure_ascii=False, separators=(",", ":"))

    def add_network(self, ssid, password):
        """Returns (status code, reply text) like handle_NewWiFiCreds."""
        if not ssid or not password:
            return 400, "Enter both the network name and the password."
        if len(ssid.encode("utf-8")) >= WIFI_SSID_LEN:
            return 400, "The network name is too long (31 bytes at most)."
        if len(password.encode("utf-8")) >= WIFI_PASS_LEN:
            return 400, "The password is too long (31 bytes at most)."
        with self.lock:
            if self.scenario in JOIN_FAIL_REASON:
                # Access point mode: the lamp hands the network to its check and saves it only when the join works.
                # A check that is running, or one that passed (the lamp restarts), refuses the next request.
                if self.join is not None and self.join_state() == "checking":
                    return 409, JOIN_BUSY_TEXT
                if self.join is not None and self.join_state() == "ok":
                    return 409, JOIN_RESTARTING_TEXT
                self.join = (ssid, time.monotonic())
                self.join_saved = False
                return 202, JOIN_ACCEPTED_TEXT
            # Station mode: the lamp scans, announces, leaves its network and restarts; a request while that runs is refused
            if self.sta_phase() in ("scanning", "announcing", "offline"):
                return 409, JOIN_BUSY_TEXT
            self.switch = (ssid, time.monotonic(), STA_FAIL_REASON.get(self.scenario))
            # The outcome of the previous switch is replaced by this one
            self.last = None
            return 202, STA_ACCEPTED_TEXT

    def join_state(self):
        """State of the mock check: idle, checking, ok or fail. The caller holds the lock."""
        if self.join is None:
            return "idle"
        if time.monotonic() - self.join[1] < JOIN_CHECK_S:
            return "checking"
        return "fail" if JOIN_FAIL_REASON[self.scenario] else "ok"

    def join_json(self):
        """Body of /api/join like the lamp builds it: ip only when ok, reason only when fail."""
        with self.lock:
            if self.sta_flow():
                return self.sta_join_json()
            state = self.join_state()
            ssid = self.join[0] if self.join else ""
            if state == "ok" and not self.join_saved:
                # The lamp saves the network when the join works
                self.join_saved = True
                self.place_first(ssid)
            body = {"state": state, "ssid": ssid}
            if state == "ok":
                body["ip"] = JOIN_IP
                left_s = JOIN_OK_HOLD_S - (time.monotonic() - self.join[1] - JOIN_CHECK_S)
                body["restart_ms"] = max(0, int(left_s * 1000))
            body["host"] = JOIN_HOST
            if state == "fail":
                body["reason"] = JOIN_FAIL_REASON[self.scenario]
            return json.dumps(body, ensure_ascii=False, separators=(",", ":"))

    def sta_join_json(self):
        """Body of /api/join in the station scenarios, with the outcome of the last switch. The caller holds the lock."""
        phase = self.sta_phase()
        state = {"scanning": "checking", "announcing": "switching", "offline": "switching", "scan_failed": "fail"}.get(phase, "idle")
        body = {"state": state, "ssid": self.switch[0] if self.switch else ""}
        body["host"] = JOIN_HOST
        if state == "fail":
            body["reason"] = self.switch[2]
        if self.last is not None:
            body["last"] = self.last
        return json.dumps(body, ensure_ascii=False, separators=(",", ":"))

    def delete_networks(self):
        with self.lock:
            if not self.networks:
                return 200, "There are no saved networks."
            if self.scenario == "errors":
                return 500, DELETE_FAIL_TEXT
            self.networks = []
        return 200, "Deleted all saved networks."


class PreviewServer(ThreadingHTTPServer):
    # On Windows SO_REUSEADDR lets a second process bind a port that is already in use
    allow_reuse_address = os.name != "nt"


class PreviewHandler(BaseHTTPRequestHandler):
    server_version = "HAQuDAPreview"
    lamp = None
    latency_s = 0.0

    def send_body(self, code, content_type, body, cors=False):
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        if cors:
            # Only /api/boot carries it, like the lamp: the page reads it cross-origin to see the lamp has restarted
            self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def send_not_found(self):
        # The lamp's onNotFound answers with an empty 404
        self.send_body(404, "text/plain", b"")

    def path_only(self):
        # The ?v= asset version and any other query are ignored, as on the lamp
        return urlsplit(self.path).path

    def drop_while_offline(self):
        """Closes the connection without an answer while the mock lamp is away. True when it did."""
        if not self.lamp.is_offline():
            return False
        self.close_connection = True
        return True

    def do_GET(self):
        if self.latency_s:
            time.sleep(self.latency_s)
        if self.drop_while_offline():
            return
        path = self.path_only()
        if path in ASSETS:
            self.send_asset(path)
        elif path == "/api/status":
            code, body = self.lamp.status_json()
            self.send_body(code, "application/json" if code == 200 else "text/plain", body)
        elif path == "/api/disp":
            code, body = self.lamp.disp_json()
            self.send_body(code, "application/json" if code == 200 else "text/plain", body)
        elif path == "/api/boot":
            self.send_body(200, "application/json", self.lamp.boot_json(), cors=True)
        elif path == "/api/wifi":
            self.send_body(200, "application/json", self.lamp.wifi_json())
        elif path == "/api/join":
            self.send_body(200, "application/json", self.lamp.join_json())
        elif path == "/webserial":
            self.send_body(200, "text/html", WEBSERIAL_STUB)
        elif self.lamp.scenario in AP_SCENARIOS:
            # The lamp's captive portal answers every unknown URL with the index page for AP clients
            self.send_asset("/")
        else:
            self.send_not_found()

    def send_asset(self, path):
        file_name, content_type = ASSETS[path]
        try:
            with open(os.path.join(SCRIPT_DIR, file_name), "rb") as source:
                data = source.read()
        except OSError:
            self.send_body(500, "text/plain", "Cannot read {0}.".format(file_name))
            return
        self.send_body(200, content_type, data)

    def read_form(self):
        """Parsed urlencoded body as {name: first value}, or None when the body is too large."""
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            length = 0
        if length < 0 or length > MAX_BODY_BYTES:
            return None
        raw = self.rfile.read(length).decode("utf-8", "replace")
        return {name: values[0] for name, values in parse_qs(raw, keep_blank_values=True).items()}

    def do_POST(self):
        if self.latency_s:
            time.sleep(self.latency_s)
        if self.drop_while_offline():
            return
        path = self.path_only()
        if path == "/api/join/dismiss":
            self.lamp.dismiss_last()
            self.send_body(204, "text/plain", b"")
        elif path == "/add_wifi_creds":
            form = self.read_form()
            if form is None:
                self.send_body(413, "text/plain", "The request is too large.")
                return
            code, text = self.lamp.add_network(form.get("ssid", ""), form.get("pass", ""))
            self.send_body(code, "text/plain", text)
        elif path == "/delete_wifi_creds":
            code, text = self.lamp.delete_networks()
            self.send_body(code, "text/plain", text)
        else:
            self.send_not_found()

    def log_message(self, fmt, *args):
        # Malformed requests (for example TLS bytes from an https:// URL) fail before command and path are set.
        # Only the method and path are printed: they carry no form data, so no password reaches the console.
        path = urlsplit(getattr(self, "path", "")).path
        sys.stderr.write("{0} {1}\n".format(getattr(self, "command", None) or "-", path))


def main():
    parser = argparse.ArgumentParser(description="Preview the HAQuDA web pages with mock data")
    parser.add_argument("--scenario", choices=SCENARIOS, default="normal",
                        help="normal; bad-air (high CO2, TVOC, PM2.5); sensor-fail (null values); ap (access point mode); "
                             "sta-offline (station not connected); errors (the lamp's 500 replies); "
                             "join-ok, join-wrong-pass, join-no-network (access point mode: a new network is checked for "
                             "about 4 s, then the result is reported); "
                             "sta-switch-ok, sta-switch-wrong-pass, sta-switch-no-network (station mode: the lamp scans, "
                             "leaves its network for about 10 s and comes back with the outcome; no-network fails at the scan); "
                             "disp-effects, disp-errors, disp-night (the picture of /api/disp cycles through the effects, "
                             "the error effects, the night bar)")
    parser.add_argument("--host", default="127.0.0.1", help="address to bind (default 127.0.0.1)")
    parser.add_argument("--port", type=int, default=8080, help="port to listen on (default 8080)")
    parser.add_argument("--latency", type=int, default=0, metavar="MS", help="delay every answer by MS milliseconds")
    parser.add_argument("--no-browser", action="store_true", help="do not open the browser")
    args = parser.parse_args()

    PreviewHandler.lamp = MockLamp(args.scenario)
    PreviewHandler.latency_s = max(0, args.latency) / 1000.0
    try:
        server = PreviewServer((args.host, args.port), PreviewHandler)
    except OSError as error:
        print("Cannot listen on {0}:{1}: {2}".format(args.host, args.port, error), file=sys.stderr)
        return 1
    server.daemon_threads = True

    # A wildcard bind address is not a browsable address: the browser goes to the loopback one
    browse_host = "127.0.0.1" if args.host in ("0.0.0.0", "::", "") else args.host
    url = "http://{0}:{1}/".format(browse_host, server.server_address[1])
    print("HAQuDA preview, scenario '{0}': {1}  (Ctrl+C stops)".format(args.scenario, url), flush=True)
    if not args.no_browser:
        webbrowser.open(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
