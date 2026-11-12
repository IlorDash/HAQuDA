#!/usr/bin/env python3
"""Local preview of the lamp's web pages with mock data, for design work in a browser on the PC.

Run from any directory:  py -3 Software/Web_Pages/preview_server.py
                         [--scenario normal|bad-air|sensor-fail|ap|sta-offline|errors|join-ok|join-wrong-pass|join-no-network]
                         [--host 127.0.0.1] [--port 8080] [--latency MS] [--no-browser]

The page sources next to this script are read from disk on every request (Cache-Control: no-store), so saving a file
and refreshing the browser shows the change. The routes, JSON shapes, status codes, content types and reply texts of
HAQuDA_WebServer.cpp are reproduced for the success paths and for these error paths: the 400 and 409 replies of
/add_wifi_creds, the 500 replies of /api/status, /add_wifi_creds and /delete_wifi_creds (scenario "errors"), and the
index page for unknown GET URLs in the access point scenarios (the captive portal). The access point scenarios ("ap"
and join-*) answer like the AP-mode firmware: POST /add_wifi_creds answers 202, /api/join reports "checking" for about 4 s and then the result of the
scenario ("ap" passes like join-ok; join-wrong-pass and join-no-network fail); a network that passed is added to the
saved list. Not reproduced: gzip, the ?v= asset
versions (the query is ignored), the restart after saving a network (the reply text is sent, nothing restarts) and the
WebSerial page. Received passwords are neither printed nor stored.
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

SCENARIOS = ["normal", "bad-air", "sensor-fail", "ap", "sta-offline", "errors", "join-ok", "join-wrong-pass", "join-no-network"]
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

# Replies of the lamp's 500 paths
STATUS_FAIL_TEXT = "Status unavailable."
READ_FAIL_TEXT = (
    "The lamp cannot read its saved networks. Try again. "
    "If it fails again, press Delete all networks on the WiFi networks page and add the network again."
)
SAVE_FAIL_TEXT = (
    "The lamp cannot save the network. Try again. "
    "If it fails again, press Delete all networks on the WiFi networks page and add the network again."
)
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
        self.failures = 0
        # The check of the last POST in a join-* scenario: (ssid, start time); None until one was started
        self.join = None
        self.join_saved = False

    def current_uptime_s(self):
        """Uptime of the mock lamp. The caller holds the lock."""
        uptime = self.uptime_offset_s + int(time.monotonic() - self.started)
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
            return '{{"fw":"{0}","uptime":{1},"host":"{2}"}}'.format(FW_VERSION, self.current_uptime_s(), JOIN_HOST.split(".")[0])

    def status_json(self):
        """Returns (status code, body)."""
        with self.lock:
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
            ssid = json.dumps(STA_SSID, ensure_ascii=False) if station and connected else "null"
            return 200, '{{"fw":"{0}","mode":"{1}","ssid":{2},"ip":"{3}","uptime":{4},"rssi":{5},"meas":{{{6}}}}}'.format(
                FW_VERSION, "sta" if station else "ap", ssid, ip, uptime, rssi, ",".join(parts)
            )

    def wifi_json(self):
        with self.lock:
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
            if self.scenario == "errors":
                # Alternate the two 500 texts of the lamp: unreadable list, unwritable record
                self.failures += 1
                return 500, (READ_FAIL_TEXT if self.failures % 2 else SAVE_FAIL_TEXT)
            if ssid in self.networks:
                # The mock keeps no passwords: only the name matters for the page
                return 200, "This network was already saved. The password is updated and the lamp restarts in 3 seconds."
            if len(self.networks) >= MAX_WIFI_CREDS_NUM:
                return 409, ("The lamp stores 10 networks and all places are taken. "
                             "Press Delete all networks on the WiFi networks page, then add this network again.")
            self.networks.append(ssid)
        return 200, "Saved. The lamp restarts in 3 seconds."

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
            state = self.join_state()
            ssid = self.join[0] if self.join else ""
            if state == "ok" and not self.join_saved:
                # The lamp saves the network when the join works
                self.join_saved = True
                if ssid not in self.networks and len(self.networks) < MAX_WIFI_CREDS_NUM:
                    self.networks.append(ssid)
            body = {"state": state, "ssid": ssid}
            if state == "ok":
                body["ip"] = JOIN_IP
                left_s = JOIN_OK_HOLD_S - (time.monotonic() - self.join[1] - JOIN_CHECK_S)
                body["restart_ms"] = max(0, int(left_s * 1000))
            body["host"] = JOIN_HOST
            if state == "fail":
                body["reason"] = JOIN_FAIL_REASON[self.scenario]
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

    def do_GET(self):
        if self.latency_s:
            time.sleep(self.latency_s)
        path = self.path_only()
        if path in ASSETS:
            self.send_asset(path)
        elif path == "/api/status":
            code, body = self.lamp.status_json()
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
        path = self.path_only()
        if path == "/add_wifi_creds":
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
                             "about 4 s, then the result is reported)")
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
