#!/usr/bin/env python3
"""Local preview of the lamp's web pages with mock data, for design work in a browser on the PC.

Run from any directory:  py -3 Software/Web_Pages/preview_server.py [--scenario normal|bad-air|sensor-fail|ap|sta-offline|errors]
                                                                    [--host 127.0.0.1] [--port 8080] [--latency MS] [--no-browser]

The page sources next to this script are read from disk on every request (Cache-Control: no-store), so saving a file
and refreshing the browser shows the change. The routes, JSON shapes, status codes, content types and reply texts of
HAQuDA_WebServer.cpp are reproduced for the success paths and for these error paths: the 400 and 409 replies of
/add_wifi_creds, the 500 replies of /api/status, /add_wifi_creds and /delete_wifi_creds (scenario "errors"), and the
index page for unknown GET URLs in the "ap" scenario (the captive portal). Not reproduced: gzip, the ?v= asset
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

SCENARIOS = ["normal", "bad-air", "sensor-fail", "ap", "sta-offline", "errors"]

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

    def status_json(self):
        """Returns (status code, body)."""
        with self.lock:
            if self.scenario == "errors":
                return 500, STATUS_FAIL_TEXT
            station = self.scenario != "ap"
            connected = self.scenario != "sta-offline"
            uptime = self.uptime_offset_s + int(time.monotonic() - self.started)
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
            return 200, '{{"fw":"{0}","mode":"{1}","ip":"{2}","uptime":{3},"rssi":{4},"meas":{{{5}}}}}'.format(
                FW_VERSION, "sta" if station else "ap", ip, uptime, rssi, ",".join(parts)
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

    def send_body(self, code, content_type, body):
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
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
        elif path == "/api/wifi":
            self.send_body(200, "application/json", self.lamp.wifi_json())
        elif path == "/webserial":
            self.send_body(200, "text/html", WEBSERIAL_STUB)
        elif self.lamp.scenario == "ap":
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
                             "sta-offline (station not connected); errors (the lamp's 500 replies)")
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
