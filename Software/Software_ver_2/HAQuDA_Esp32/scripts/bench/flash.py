"""Flash: finds esptool and the toolchain of the installed esp32 core, hands the port over from the
monitor, and writes the last bench build with esptool."""
import codecs
import ctypes
import os
import re
import subprocess
import time
from collections import namedtuple
from datetime import datetime

from . import output, winapi
from .project import PLATFORM, PROJ, first_line, read_latin1, same_path

EspTools = namedtuple("EspTools", "core_path core_version esptool toolchain_prefix")
HEAD = "Writing at "  # esptool's progress lines


def _version_key(name):
    return tuple(int(n) for n in name.split(".")) if re.fullmatch(r"\d+(\.\d+)*", name) else (0,)


def _subdirs(path):
    return [d for d in sorted(os.listdir(path)) if os.path.isdir(os.path.join(path, d))] if os.path.isdir(path) else []


def _documents_dir():
    buf = ctypes.create_unicode_buffer(260)
    ctypes.windll.shell32.SHGetFolderPathW(None, 5, None, 0, buf)  # CSIDL_PERSONAL
    return buf.value


def find_esp_tools(out_dir):
    """The esp32 core folder, esptool.exe and the xtensa toolchain prefix ("...\\bin\\xtensa-esp32-elf-",
    None when not found). The core is the one the last build compiled against (its folder is named in
    the dependency files of the sketch objects); without that evidence the highest installed version
    is used and a warning says so."""
    bases = [os.path.join(_documents_dir(), "ArduinoData", "packages", "esp32"),
             os.path.join(os.environ.get("USERPROFILE", ""), "Documents", "ArduinoData", "packages", "esp32"),
             os.path.join(os.environ.get("LOCALAPPDATA", ""), "Arduino15", "packages", "esp32")]
    roots = []
    for base in bases:
        if os.path.isdir(base) and not any(same_path(r, base) for r in roots):
            roots.append(base)
    cores = [(r, v, os.path.join(r, "hardware", "esp32", v)) for r in roots for v in _subdirs(os.path.join(r, "hardware", "esp32"))]
    if not cores:
        raise RuntimeError("No esp32 core found under ArduinoData\\packages or Arduino15\\packages.")

    used = None
    sketch_out = os.path.join(out_dir, "sketch")
    deps = [f for f in sorted(os.listdir(sketch_out)) if f.endswith(".d")] if os.path.isdir(sketch_out) else []
    if deps:
        with open(os.path.join(sketch_out, deps[0]), encoding="utf-8", errors="replace") as f:
            m = re.search(r"hardware[\\/]+esp32[\\/]+(\d[^\\/\s]*)", f.read())
        used = m.group(1) if m else None
    core = next((c for c in cores if c[1] == used), None) if used else None
    if core is None:
        core = max(cores, key=lambda c: _version_key(c[1]))
        if len(cores) > 1 or used:
            output.warn("Could not tell which esp32 core the build used; using the highest installed one (%s)." % core[1])

    # The core's own package root first, then the others.
    search = [core[0]] + [r for r in roots if not same_path(r, core[0])]
    esptool = None
    for root in search:
        tool_dir = os.path.join(root, "tools", "esptool_py")
        found = [d for d in _subdirs(tool_dir) if os.path.isfile(os.path.join(tool_dir, d, "esptool.exe"))]
        if found:
            esptool = os.path.join(tool_dir, max(found, key=_version_key), "esptool.exe")
            break
    if esptool is None:
        raise RuntimeError("esptool.exe not found under packages\\esp32\\tools\\esptool_py.")
    prefix = None
    for root in search:
        gcc = os.path.join(root, "tools", "xtensa-esp32-elf-gcc")
        found = [d for d in _subdirs(gcc) if os.path.isfile(os.path.join(gcc, d, "bin", "xtensa-esp32-elf-addr2line.exe"))]
        if found:
            prefix = os.path.join(gcc, max(found, key=_version_key), "bin", "xtensa-esp32-elf-")
            break
    return EspTools(core[2], core[1], esptool, prefix)


def project_property(text, key):
    m = re.search(r"<Key>%s</Key>\s*<Value>([^<]*)</Value>" % key, text)
    if not m:
        raise RuntimeError("Board property %s not found in the bench project; set it in the VisualGDB project properties." % key)
    return m.group(1).strip()


def board_menu_value(boards_file, menu, choice, prop):
    """esp32.menu.<menu>.<choice>.<prop>=<value> from boards.txt."""
    prefix = "esp32.menu.%s.%s.%s=" % (menu, choice, prop)
    with open(boards_file, encoding="utf-8", errors="replace") as f:
        for text in f:
            if text.startswith(prefix):
                return text[len(prefix):].strip()
    raise RuntimeError("boards.txt has no %s entry; the board property '%s' = '%s' is not supported by the script." % (prefix, menu, choice))


def command_text(exe, args):
    return " ".join(['"%s"' % exe] + ['"%s"' % a if re.search(r"\s", a) else a for a in args])


# ---- esptool output with a time prefix on every line --------------------------------------------

class EsptoolPrinter:
    """Console text for esptool output. esptool only uses CR for in-place progress when its output is
    a terminal; here it is a pipe, so the progress is rebuilt from the lines. Partial lines (the dots of
    "Connecting....") are written at once. A CR starts a new segment on the same line with a fresh prefix,
    and so does a "Writing at" line on a line of its own.
    State: is_open = text of the line is on screen; length = characters of the current segment; visible =
    the widest the line has been (a shorter rewrite is padded to it); held = a "Writing at" line ended with
    LF and stays on screen to be rewritten by the next one; buf = first characters of the line after it,
    held back until it is known whether it is another "Writing at" line; head = first characters of the
    current segment."""

    def __init__(self):
        self.pending_cr = False
        self._reset()

    def _reset(self):
        self.is_open, self.held, self.length, self.visible, self.buf, self.head = False, False, 0, 0, "", ""

    def _pad(self):
        return " " * (self.visible - self.length)

    def _start_line(self, now):
        """The held-back buf becomes a new line of its own."""
        out = "\n" + output.time_prefix(now) + self.buf
        self.is_open, self.held, self.length, self.visible, self.head, self.buf = True, False, len(self.buf), len(self.buf), self.buf, ""
        return out

    def feed(self, text, now):
        out = []
        for ch in text:
            if self.pending_cr:
                self.pending_cr = False
                if ch != "\n" and self.is_open:  # the segment ends here, the next one replaces it on the same line
                    out.append(self._pad() + "\r")
                    self.is_open, self.length, self.head = False, 0, ""
            if ch == "\r":
                self.pending_cr = True
            elif ch == "\n":
                if self.buf:
                    out.append(self._start_line(now))
                if self.is_open:
                    out.append(self._pad())  # a shorter segment must not leave the tail of a longer one
                    if self.head == HEAD:  # a progress line stays on screen, the next one may replace it
                        self.held, self.is_open = True, False
                        continue
                out.append("\n")
                self._reset()
            elif self.held and not self.is_open:
                self.buf += ch
                if HEAD.startswith(self.buf):
                    if len(self.buf) == len(HEAD):  # another progress line: rewrite the one on screen
                        out.append(self._pad() + "\r" + output.time_prefix(now) + self.buf)
                        self.is_open, self.held, self.length, self.head, self.buf = True, False, len(self.buf), self.buf, ""
                else:  # something else: the progress line stays and this starts a new line
                    out.append(self._start_line(now))
            else:
                if not self.is_open:
                    out.append(output.time_prefix(now))
                    self.is_open, self.length, self.head = True, 0, ""
                out.append(ch)
                self.length += 1
                self.visible = max(self.visible, self.length)
                if len(self.head) < len(HEAD):
                    self.head += ch
        return "".join(out)

    def end(self, now):
        """Ends the last line when esptool has finished."""
        out = self._start_line(now) if self.buf else ""
        if self.is_open or self.held:
            out += "\n"
        self._reset()
        return out


def run_esptool(exe, args):
    """Runs esptool with its output re-printed with the time prefix; returns its exit code. Python
    buffers a pipe, so unbuffered output is requested and the dots of "Connecting...." appear as printed."""
    proc = subprocess.Popen([exe] + args, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            env=dict(os.environ, PYTHONUNBUFFERED="1", PYTHONIOENCODING="utf-8"), bufsize=0,
                            creationflags=subprocess.CREATE_NO_WINDOW)
    winapi.adopt(proc)  # a killed terminal must not leave esptool running after the lock goes stale
    printer, decoder = EsptoolPrinter(), codecs.getincrementaldecoder("utf-8")("replace")
    try:
        chunk = proc.stdout.read(2048)
        while chunk:
            output.write(printer.feed(decoder.decode(chunk), datetime.now()))
            chunk = proc.stdout.read(2048)
        proc.wait()
        output.write(printer.end(datetime.now()))
        return proc.returncode
    finally:
        if proc.poll() is None:
            proc.kill()


# ---- the port hand-over with the monitor --------------------------------------------------------

def wait_monitor_release(bench):
    """Flash side: the monitor creates <bench>\\monitor.released once its port is closed."""
    if winapi.record_state(os.path.join(bench, "monitor.pid")) != "alive":
        return
    ack, start = os.path.join(bench, "monitor.released"), time.monotonic()
    while not os.path.isfile(ack):
        if time.monotonic() - start >= 5:
            output.warn("The monitor did not confirm that it released the port; flashing anyway.")
            return
        time.sleep(0.1)
    time.sleep(0.3)  # the driver needs a moment after the close before another program can open the port


def run_flash(bench, cfg, port, not_older_than, dry_run):
    """not_older_than: wall time (Build + Flash); the app image must have been written after it,
    otherwise the build did not produce a new image and an old one would be flashed."""
    if not port:
        raise RuntimeError("No COM port is set. Run the Change COM port task.")
    proj_file = os.path.join(bench, PROJ)
    if not os.path.isfile(proj_file):
        raise RuntimeError("The bench has no project yet. Build first.")
    text = read_latin1(proj_file)
    out_dir = os.path.join(bench, "build", PLATFORM, cfg)
    m = re.search(r"<MainSketchFile>([^<]+)</MainSketchFile>", text)
    if not m:
        raise RuntimeError("MainSketchFile not found in " + proj_file)
    bootloader, partitions, app = (os.path.join(out_dir, "%s.%s" % (m.group(1), s)) for s in ("bootloader.bin", "partitions.bin", "bin"))
    for f in (bootloader, partitions, app):
        if not os.path.isfile(f):
            raise RuntimeError("Nothing to flash: %s is missing. Build first (task Build, or Build + Flash + Monitor)." % f)
    built = os.stat(app).st_mtime
    if not_older_than is not None and built < not_older_than:
        raise RuntimeError("The build did not produce a new image, not flashing.")
    output.trace("flash: check images")

    tools = find_esp_tools(out_dir)
    output.trace("flash: find_esp_tools")
    boot_app, boards = (os.path.join(tools.core_path, p) for p in ("tools\\partitions\\boot_app0.bin", "boards.txt"))
    for f in (boot_app, boards):
        if not os.path.isfile(f):
            raise RuntimeError("%s is missing; the esp32 core looks incomplete." % f)
    value = lambda menu, prop: board_menu_value(boards, menu, project_property(text, menu), prop)
    speed, mode = value("UploadSpeed", "upload.speed"), value("FlashMode", "build.flash_mode")
    freq, size = value("FlashFreq", "build.flash_freq"), value("FlashSize", "build.flash_size")
    args = ["--chip", "esp32", "--port", port, "--baud", speed, "--before", "default_reset", "--after", "hard_reset",
            "write_flash", "-z", "--flash_mode", mode, "--flash_freq", freq, "--flash_size", size,
            "0x1000", bootloader, "0x8000", partitions, "0xe000", boot_app, "0x10000", app]
    output.trace("flash: board settings")
    output.line("Flashing the build of %s (%s) to %s at %s baud, esp32 core %s." % (
        first_line(os.path.join(bench, "last-source.txt")), datetime.fromtimestamp(built).strftime("%Y-%m-%d %H:%M:%S"),
        port, speed, tools.core_version))
    if dry_run:
        output.line(command_text(tools.esptool, args))
        return 0

    lock, ack = os.path.join(bench, "flashing.lock"), os.path.join(bench, "monitor.released")
    if winapi.lock_held(lock):
        raise RuntimeError("Another flash is running on this bench.")
    winapi.remove(ack)  # an ack left by an earlier flash would let esptool start before the monitor released the port
    winapi.write_record(lock)
    try:
        wait_monitor_release(bench)  # the monitor closes the port when it sees the lock; esptool must not start before
        code = run_esptool(tools.esptool, args)
    finally:
        if winapi.record_pid(lock) == str(os.getpid()):
            winapi.remove(lock)
    if code != 0:
        output.line("esptool failed (exit code %d). If it could not open %s: the port is busy (is another program using it, "
                    "for example a terminal in Visual Studio?) or it is the wrong port. Pick another with the Change COM port "
                    "task. If it could not connect: the lamp must be in the bootloader mode when the flash starts." % (code, port))
    else:
        output.line("Flash done. Reset the lamp if it does not restart by itself.")
    return code
