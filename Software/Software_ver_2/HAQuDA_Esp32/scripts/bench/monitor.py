"""Serial monitor of the lamp: esp-idf-monitor plus the port hand-over to the flash step.

Needs esp-idf-monitor: py -3 -m pip install esp-idf-monitor

The wrapper does what esp-idf-monitor cannot: while <bench>\\flashing.lock names a live process (the
flash step writes it) it stops the monitor, so esptool can open the port, and writes
<bench>\\monitor.released as the acknowledgement. When the lock is gone it starts the monitor again;
this also picks up a new ELF. It does the same when <bench>\\port names another port. One wrapper
runs per bench (<bench>\\monitor.pid, same record), a second one exits at once.

Quit with Ctrl+] (esp-idf-monitor's own key) or Ctrl+C; its Ctrl+T menu is not supported (VS Code
takes most Ctrl+<key> presses). A missing or busy port is reported by
esp-idf-monitor, which then keeps waiting for it (--open-port-attempts 0).

The "---" lines are printed without a time prefix: the VS Code task matches them at the line start.
"""
import importlib.util
import os
import re
import subprocess
import sys
import time

from . import output, winapi
from .flash import find_esp_tools
from .project import PLATFORM, PROJ, first_line, read_latin1

POLL_S = 0.2
POLL_HELD_S = 0.05


def say(text):
    print(text, flush=True)


def monitor_command(port, baud, elf, prefix):
    # The \\.\COMx form is what esp-idf-monitor's GDB check accepts; a plain COMx makes it warn.
    command = [sys.executable, "-m", "esp_idf_monitor", "--port", "\\\\.\\" + port, "--baud", str(baud), "--no-reset",
               "--timestamps", "--timestamp-format", "%H:%M:%S.%f |", "--target", "esp32",
               # 0 = infinite attempts to wait for the port to appear: a missing port is reported once and
               # the monitor keeps waiting, instead of exiting with the same code as a quit.
               "--open-port-attempts", "0"]
    # Address decoding needs the ELF of the last build and addr2line of the toolchain.
    if prefix and os.path.isfile(elf):
        return command + ["--toolchain-prefix", prefix, elf]
    return command + ["--disable-address-decoding"]


def stop(child, modes):
    child.terminate()
    try:
        child.wait(3)
    except subprocess.TimeoutExpired:
        child.kill()
        child.wait()
    winapi.restore_console(modes)


def run(bench, port, baud, elf, prefix):
    """Supervises esp-idf-monitor until the user quits it; returns its exit code."""
    lock, ack, port_file = (os.path.join(bench, n) for n in ("flashing.lock", "monitor.released", "port"))
    modes, child, announced = winapi.console_modes(), None, False
    try:
        while True:
            # Before the lock check: a quit in the same tick as a flash lock must end the wrapper.
            if child and child.poll() is not None:
                winapi.restore_console(modes)  # the user quit (Ctrl+]) or esp-idf-monitor failed; it reports why
                code, child = child.returncode, None
                return code
            wanted = (first_line(port_file) or "").upper()
            if wanted and wanted != port:
                port = wanted
                say("--- port changed to %s ---" % port)
                if child:
                    stop(child, modes)
                    child = None
            held = winapi.lock_held(lock)
            if held:
                if child:
                    stop(child, modes)
                    child = None
                    say("--- port released for flashing ---")
                if not os.path.exists(ack):
                    with open(ack, "w", encoding="ascii") as f:
                        f.write("released")
            else:
                winapi.remove_quiet(ack)
                if child is None:
                    winapi.flush_console_input()
                    child = subprocess.Popen(monitor_command(port, baud, elf, prefix))
                    winapi.adopt(child)
                    if not announced:
                        say("--- monitor ready, %s at %s baud, quit with Ctrl+] ---" % (port, baud))
                        # VS Code takes most Ctrl+<key> presses for its own commands, so the menu
                        # announced by esp-idf-monitor is unusable there.
                        say(output.paint("--- Ctrl+T menu commands do not work in this terminal; "
                                         "only Ctrl+] and Ctrl+C (quit) do ---", "31"))
                        announced = True
            # Fast while the lock is held: the monitor restarts right when the flash ends, and the
            # first boot lines of the new firmware (its version) must not be lost.
            time.sleep(POLL_HELD_S if held else POLL_S)
    finally:
        if child:
            stop(child, modes)


def run_monitor(main_fw, bench, port, cfg, dry_run):
    if not port:
        raise RuntimeError("No COM port is set. Run the Change COM port task.")
    # The bench project when it exists; the main project is the same text and exists before the first build.
    proj_file = os.path.join(bench, PROJ)
    if not os.path.isfile(proj_file):
        proj_file = os.path.join(main_fw, PROJ)
    if not os.path.isfile(proj_file):
        raise RuntimeError("VisualGDB project not found: " + proj_file)
    text = read_latin1(proj_file)
    baud = re.search(r"<BaudRate>(\d+)</BaudRate>", text)  # the baud rate of VisualGDB's terminal for this lamp
    sketch = re.search(r"<MainSketchFile>([^<]+)</MainSketchFile>", text)
    if not baud:
        raise RuntimeError("BaudRate not found in the VisualGDB project.")
    if not sketch:
        raise RuntimeError("MainSketchFile not found in the VisualGDB project.")

    # The ELF of the last build and the toolchain turn the addresses of a crash into source lines.
    out_dir = os.path.join(bench, "build", PLATFORM, cfg)
    elf, prefix = os.path.join(out_dir, sketch.group(1) + ".elf"), None
    try:
        prefix = find_esp_tools(out_dir).toolchain_prefix
    except RuntimeError as e:
        output.warn(str(e))
    if not prefix:
        output.warn("xtensa toolchain not found: crash addresses will not be decoded.")
    port, baud = port.upper(), baud.group(1)
    if dry_run:
        say(subprocess.list2cmdline(monitor_command(port, baud, elf, prefix)))
        return 0
    if importlib.util.find_spec("esp_idf_monitor") is None:
        say("esp-idf-monitor is not installed. Install it with:")
        say("  py -3 -m pip install esp-idf-monitor")
        return 1
    pid_file = os.path.join(bench, "monitor.pid")
    if winapi.record_state(pid_file) == "alive":
        say("A monitor of this bench is already running in another terminal. This one exits.")
        return 0

    winapi.write_record(pid_file)
    say("--- monitor starting ---")
    code = 0
    try:
        code = run(bench, port, baud, elf, prefix)
    except KeyboardInterrupt:
        pass
    finally:
        if winapi.record_pid(pid_file) == str(os.getpid()):
            winapi.remove_quiet(pid_file)
        winapi.remove_quiet(os.path.join(bench, "monitor.released"))
        say("--- monitor stopped ---")
    return code
