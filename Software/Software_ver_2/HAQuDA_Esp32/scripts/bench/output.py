"""Console output of haquda-bench.py: time-prefixed lines, colours, progress line, spinner."""
import contextlib
import shutil
import sys
import threading
from datetime import datetime

from . import winapi

ESC = "\x1b"
SPIN = "|/-\\"
PREFIX_WIDTH = 15  # "HH:MM:SS.fff | "
_ansi = False
_inline = False  # a progress line is on screen


def init():
    """Escape sequences are used only on an interactive console that can process them."""
    global _ansi
    try:
        sys.stdout.reconfigure(errors="replace")  # a legacy code page must not fail on a path
    except (AttributeError, ValueError):
        pass
    _ansi = sys.stdout.isatty() and winapi.enable_vt()


def write(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def time_prefix(now=None):
    """'HH:MM:SS.fff | ' with the time dim."""
    ts = (now or datetime.now()).strftime("%H:%M:%S.%f")[:-3]
    return "%s[2m%s |%s[0m " % (ESC, ts, ESC) if _ansi else ts + " | "


def paint(text, color):
    """text in an ANSI colour ('31' red, '33' yellow) if escapes are on, else as is."""
    return "%s[%sm%s%s[0m" % (ESC, color, text, ESC) if color and _ansi else text


def line(text="", color=""):
    """The one way to print a line; color as in paint()."""
    if not text:
        write("\n")
        return
    write(time_prefix() + paint(text, color) + "\n")


def warn(text):
    line("WARNING: " + text, "33")


def width():
    """Columns of a console line that never wrap: one less than the window."""
    return max(shutil.get_terminal_size((80, 24)).columns - 1, 1)


def progress_width():
    return max(10, width() - PREFIX_WIDTH)


def progress(text):
    """Redraws the in-place progress line; the time shown is the time of this rewrite."""
    global _inline
    write("\r" + time_prefix() + text)
    _inline = True


def clear_progress():
    global _inline
    if _inline:
        write("\r" + " " * width() + "\r")
        _inline = False


@contextlib.contextmanager
def spinner(label, plain_line=True):
    """An in-place '<label> |' line that animates on its own thread when the step takes longer than
    0.3 s. The step must not print. Redirected output gets a plain line (or nothing) instead."""
    if not sys.stdout.isatty():
        if plain_line:
            line(label + "...")
        yield
        return
    stop, drawn = threading.Event(), []

    def run():
        tick = 0
        if stop.wait(0.3):
            return
        while not stop.is_set():
            try:
                room = progress_width()
                write("\r" + time_prefix() + ("%s %s" % (label, SPIN[tick % 4]))[:room].ljust(room))
                drawn.append(1)
            except OSError:
                pass  # a console that cannot be written to must not take the process down
            tick += 1
            stop.wait(0.1)

    thread = threading.Thread(target=run, daemon=True)
    thread.start()
    try:
        yield
    finally:
        stop.set()
        thread.join(1)
        if drawn:
            write("\r" + " " * width() + "\r")
