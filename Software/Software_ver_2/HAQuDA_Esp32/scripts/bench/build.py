"""Build: runs VisualGDB's builder on the bench project with a live progress line. The builder log
holds secret -D values, so it is never printed: only allow-listed lines reach the console."""
import math
import os
import re
import subprocess
import sys
import time
import winreg

from . import output, winapi
from .project import BUILD_LOCK, PLATFORM, PROJ, ensure_bench, first_line, write_text

# Builder stage messages that may be shown while the build runs. The builder logs them as
# "===info ||| <template> ||| [<arguments>]". A strict allow-list on the whole template: any line
# that is not exactly one of these is never printed.
STAGES = ("Detecting libraries used...", "Generating function prototypes...", "Compiling sketch...",
          "Compiling libraries...", "Compiling core...", "Linking everything together...")
LIBRARY_TEMPLATE = 'Compiling library "{0}"'
SIZE_TEMPLATES = ("Sketch uses {0} bytes ({2}%%) of program storage space. Maximum is {1} bytes.",
                  "Global variables use {0} bytes ({2}%%) of dynamic memory, leaving {3} bytes for local variables. "
                  "Maximum is {1} bytes.")
# An error line is shown only when it looks like a compiler diagnostic: "<file>:<line>:<col>: error:"
# or "fatal error:". Anything else with "error:" in it (a command line, an argument dump) is hidden.
DIAGNOSTIC = re.compile(r"(?:(?:[A-Za-z]:)?[^:\r\n]+:\d+:\d+: (?:fatal )?error: |fatal error: )")


def refuse_while_building(bench):
    """Raises when a build of another process holds the bench; Flash and Clean must wait for it."""
    if winapi.held_by_other(os.path.join(bench, BUILD_LOCK)):
        raise RuntimeError("A build is running on this bench. Wait for it to end, then run again.")


def take_bench_lock(bench):
    """Takes building.lock in an existing bench. Raises when another live process holds it."""
    if not winapi.acquire_lock(os.path.join(bench, BUILD_LOCK)):
        raise RuntimeError("The bench is busy: a build or a Remove bench is running. Wait for it to end, then run again.")


def acquire_build_lock(bench):
    """Takes the bench for a build, a project or port change. The folders and the marker are created
    only under the same guard that Clean holds while it deletes the bench, so a bench that Clean is
    removing is never recreated underneath it. Raises when another live process holds the bench, before
    anything else in it is touched."""
    with winapi.exclusive(os.path.join(bench, BUILD_LOCK)):
        ensure_bench(bench)
        take_bench_lock(bench)


def release_build_lock(bench):
    winapi.release_lock(os.path.join(bench, BUILD_LOCK))


def refuse_while_flashing(bench):
    """Raises when a flash of another process runs: a build would overwrite the images esptool reads.
    Called with the build lock held; Flash takes its lock first and checks the build lock after, so
    of a build and a flash that start together at least one sees the other and refuses."""
    if winapi.held_by_other(os.path.join(bench, "flashing.lock")):
        raise RuntimeError("A flash is running on this bench. Wait for it to end, then run the build again.")


def find_visualgdb():
    for env in ("ProgramFiles(x86)", "ProgramFiles"):
        exe = os.path.join(os.environ.get(env, ""), "Sysprogs", "VisualGDB", "VisualGDB.exe")
        if os.environ.get(env) and os.path.isfile(exe):
            return exe
    for sub in (r"SOFTWARE\WOW6432Node\Sysprogs\VisualGDB", r"SOFTWARE\Sysprogs\VisualGDB"):
        for name in ("InstallationPath", "InstallDir"):
            try:
                with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, sub) as key:
                    exe = os.path.join(winreg.QueryValueEx(key, name)[0], "VisualGDB.exe")
            except OSError:
                continue
            if os.path.isfile(exe):
                return exe
    raise RuntimeError("VisualGDB.exe not found. Install VisualGDB or add its folder to the script.")


def suspicious(text):
    """True for a line that may carry a build secret or a command line, whatever else it matches."""
    return "-D" in text or '="' in text or "Arguments" in text or "-prefs=" in text or len(text) > 200


def split_line(text):
    """(template, argument text) of a builder log line; the plain form has no arguments."""
    text = text.strip()
    if text.startswith("===info ||| "):
        text = text[12:]
    m = re.match(r"(.*?) \|\|\| \[(.*)\]$", text)
    return (m.group(1), m.group(2)) if m else (text, "")


def stage_text(text):
    """The text of an allow-listed stage line, None for every other line."""
    if suspicious(text):
        return None
    template, args = split_line(text)
    if template in STAGES:
        return template
    if template == LIBRARY_TEMPLATE and re.match(r"[A-Za-z0-9_. +-]{1,80}\Z", args):
        return "Compiling library " + args
    return None


def size_text(text):
    """Readable text of the two memory summary lines, None for any other line or non-numeric arguments."""
    if suspicious(text):
        return None
    template, args = split_line(text)
    values = args.split()
    if template not in SIZE_TEMPLATES or not all(re.match(r"[0-9]{1,12}\Z", v) for v in values):
        return None
    if len(values) <= max(int(i) for i in re.findall(r"\{(\d)\}", template)):
        return None
    return re.sub(r"\{(\d)\}", lambda m: values[int(m.group(1))], template).replace("%%", "%")


class LogTail:
    """The complete lines appended to a growing log file since the last call. A last line without
    its newline waits for the next call; read problems are ignored (the log is only a progress source)."""

    def __init__(self, path):
        self.path, self.pos, self.partial = path, 0, ""

    def read(self, flush=False):
        try:
            with open(self.path, "rb") as f:  # shared mode, the writer is not disturbed
                if os.fstat(f.fileno()).st_size < self.pos:
                    self.pos, self.partial = 0, ""
                f.seek(self.pos)
                data = f.read()
        except OSError:
            return []
        self.pos += len(data)
        parts = (self.partial + data.decode("latin-1")).split("\n")
        self.partial = parts.pop()
        if flush and self.partial:
            parts.append(self.partial)
            self.partial = ""
        return [p.rstrip("\r") for p in parts]


def last_build_seconds(bench, cfg):
    """Seconds the last successful build of this bench and configuration took; 0 when unknown."""
    m = re.match(r"(\w+) ([0-9]{1,6}(?:\.[0-9]{1,3})?)\Z", first_line(os.path.join(bench, "last-build-seconds")) or "")
    return float(m.group(2)) if m and m.group(1) == cfg else 0.0


def progress_text(tick, elapsed, expected, objects, area, width):
    """For example "Building |  23 s  ~45 %  12 objects  libraries\\WiFi". expected <= 0: no earlier build, no percent."""
    text = "Building %s %3.0f s" % (output.SPIN[tick % 4], elapsed)
    if expected > 0:
        text += "  ~%2d %%" % int(min(99, math.floor(elapsed / expected * 100)))
    text += "  %d objects" % objects
    if area:
        text += "  " + area
    return text[:width].ljust(width)  # padded so the rest of a longer earlier line is erased


def object_progress(build_dir, since):
    """(count, area) of the object files written since the time `since`, and where the newest one lies
    (first two folder levels below the build folder, for example "libraries\\WiFi"). Folder names carry
    no secrets; works even when the builder logs nothing."""
    count, newest, area = 0, 0.0, ""
    for dirpath, _, files in os.walk(build_dir):
        for name in files:
            if not name.endswith(".o"):
                continue
            try:
                mtime = os.stat(os.path.join(dirpath, name)).st_mtime
            except OSError:
                continue  # a file that vanishes under the builder is not an error for a progress line
            if mtime < since:
                continue
            count += 1
            if mtime > newest:
                newest = mtime
                levels = os.path.relpath(dirpath, build_dir).split("\\")
                # Skip the platform and configuration folders: build\<platform>\<config>\<area>.
                area = "\\".join(levels[2:4])
    return count, area


def run_build(bench, cfg):
    """Returns the builder's exit code."""
    log, err_log = (os.path.join(bench, "build", "%s-build%s.log" % (cfg, s)) for s in ("", ".err"))
    command = [find_visualgdb(), "/build", os.path.join(bench, PROJ), "/config:" + cfg, "/platform:" + PLATFORM]
    start = time.monotonic()
    start_wall = time.time()
    # A full build takes minutes; the log is followed while the builder runs so the terminal is not silent.
    with open(log, "wb") as out, open(err_log, "wb") as err:
        proc = subprocess.Popen(command, stdin=subprocess.DEVNULL, stdout=out, stderr=err)
    winapi.adopt(proc)  # the builder tree dies with this process: no orphan writes into the shared build folder
    output.line("Building. Full log goes to a file (it holds build secrets and is never shown).")
    tail, build_dir = LogTail(log), os.path.join(bench, "build")
    expected = last_build_seconds(bench, cfg)
    # A redirected output (a log file, a pipe) cannot rewrite a line: it gets a plain line every 10 s.
    inline = sys.stdout.isatty()
    last_print, count, area, next_scan, tick = 0.0, 0, "", 0.0, 0

    def show(lines):
        nonlocal last_print
        for text in lines:
            stage = stage_text(text)
            if stage:
                last_print = time.monotonic() - start
                output.clear_progress()
                output.line("[%4.0f s] %s" % (last_print, stage))

    try:
        while proc.poll() is None:
            time.sleep(0.25)
            show(tail.read())
            elapsed = time.monotonic() - start
            if elapsed >= next_scan:  # listing the object files costs more than a redraw: about once a second
                next_scan = elapsed + 1.0
                count, area = object_progress(build_dir, start_wall)
            if inline:
                tick += 1
                output.progress(progress_text(tick, elapsed, expected, count, area, output.progress_width()))
            elif elapsed - last_print >= 10:
                last_print = elapsed
                output.line("[%4.0f s] still building: %d objects compiled%s" % (elapsed, count, ", now in " + area if area else ""))
    finally:
        output.clear_progress()
        if proc.poll() is None:  # Ctrl+C or an error: the job kills the rest of the builder tree at exit
            proc.kill()
    show(tail.read(flush=True))
    seconds = time.monotonic() - start
    count = object_progress(build_dir, start_wall)[0]

    # Only selected lines are shown; a line that looks like a command line is never echoed.
    matched = []
    for path in (log, err_log):
        with open(path, "rb") as f:
            matched += [t.rstrip("\r") for t in f.read().decode("latin-1").split("\n")
                        if "error:" in t or "Sketch uses" in t or "Global variables" in t]
    for text in matched:
        readable = size_text(text)
        if readable:
            output.line(readable)
        elif suspicious(text) or ("error:" in text and not DIAGNOSTIC.match(text)):
            output.line("<line suppressed: may contain build secrets>")
        else:
            output.line(text)
    code = proc.returncode
    output.line("Build exit code %d, %.1f s, %d objects compiled" % (code, seconds, count))
    if code == 0:
        write_text(os.path.join(bench, "last-build-seconds"), "%s %.1f" % (cfg, seconds))
    elif not matched:
        output.line("Build failed, no error lines matched. Full log: %s (it contains build secrets - do not share it)" % log)
    return code
