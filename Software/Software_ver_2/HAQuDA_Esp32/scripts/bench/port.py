"""COM port: which ports exist, the menu, and the stored choice in <bench>\\port."""
import os
import re
import winreg

from . import output
from .project import PROJ, ask, first_line, read_latin1, write_text


def project_port(proj_path):
    if not os.path.isfile(proj_path):
        return None
    m = re.search(r"<COMPort>(COM\d+)</COMPort>", read_latin1(proj_path))
    return m.group(1) if m else None


def present_port_names():
    """Names of the ports that exist now (the serial device map, as .NET's GetPortNames reads it)."""
    names = set()
    try:
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"HARDWARE\DEVICEMAP\SERIALCOMM") as key:
            for i in range(winreg.QueryInfoKey(key)[1]):
                value = str(winreg.EnumValue(key, i)[1]).strip().upper()
                if re.fullmatch(r"COM\d+", value):
                    names.add(value)
    except OSError:
        pass
    return sorted(names, key=lambda n: int(n[3:]))


def _subkeys(key):
    try:
        for i in range(winreg.QueryInfoKey(key)[0]):
            yield winreg.EnumKey(key, i)
    except OSError:
        return


def _open(key, name):
    try:
        return winreg.OpenKey(key, name)
    except OSError:
        return None


def friendly_names():
    """Every FriendlyName below Enum that names a COM port ("USB Serial Port (COM10)"), devices that
    are present first (they have a Control subkey). The registry is fast; WMI takes 10-15 s here."""
    present, remembered = [], []

    def walk(key, depth):  # Enum\<bus>\<device>\<instance>; a key that cannot be read skips only itself
        for name in _subkeys(key):
            sub = _open(key, name)
            if sub is None:
                continue
            with sub:
                if depth < 2:
                    walk(sub, depth + 1)
                    continue
                try:
                    friendly = winreg.QueryValueEx(sub, "FriendlyName")[0]
                except OSError:
                    continue
                if isinstance(friendly, str) and "(COM" in friendly:
                    control = _open(sub, "Control")
                    (remembered if control is None else present).append(friendly)
                    if control is not None:
                        control.close()

    root = _open(winreg.HKEY_LOCAL_MACHINE, r"SYSTEM\CurrentControlSet\Enum")
    if root is not None:
        with root:
            walk(root, 0)
    return present + remembered


def present_ports():
    """(name, label) of the ports that exist now, label as Windows shows it; for the menu only."""
    names = present_port_names()
    output.trace("port names (SERIALCOMM)")
    friendly = {}
    with output.spinner("Looking up COM ports"):
        found = friendly_names()
    for text in found:  # the first name per port wins
        m = re.search(r"\((COM\d+)\)", text, re.I)
        if m:
            friendly.setdefault(m.group(1).upper(), text)
    output.trace("port names (registry)")
    return [(n, friendly.get(n, n)) for n in names]


def select_port_menu(current):
    ports = present_ports()
    if not ports:
        raise RuntimeError("No COM ports found. Connect the lamp over USB and run again.")
    default = 0
    for i, (name, _) in enumerate(ports, 1):
        if current and name.upper() == current.upper():
            default = i
    output.line("COM ports present:")
    for i, (_, label) in enumerate(ports, 1):
        output.line("%s [%d] %s" % ("*" if i == default else " ", i, label))
    answer = ask("Number (Enter = %d)" % default if default else "Number")
    if not answer and default:
        return ports[default - 1][0]
    if answer.isdigit() and 1 <= int(answer) <= len(ports):
        return ports[int(answer) - 1][0]
    if re.fullmatch(r"COM\d+", answer, re.I):
        return answer.upper()
    raise RuntimeError("Not a valid choice: " + answer)


def save_port(bench, port, main_port):
    write_text(os.path.join(bench, "port"), port)
    # Remembers which port the VisualGDB project named when the choice was made, to notice a later change.
    if main_port:
        write_text(os.path.join(bench, "port-origin"), main_port)


def resolve_port(main_fw, bench, explicit, interactive, force_menu):
    """The port to use; None for a Build with nothing stored and no port in the main project.
    interactive: ask questions (Build never asks, it only patches what is stored)."""
    main_port = project_port(os.path.join(main_fw, PROJ))
    stored = first_line(os.path.join(bench, "port"))
    origin = first_line(os.path.join(bench, "port-origin"))

    if explicit:
        explicit = explicit.upper()
        save_port(bench, explicit, main_port)
        if explicit not in present_port_names():
            output.warn("%s is not present now." % explicit)
        return explicit

    if not stored:
        stored = main_port
        if stored:
            save_port(bench, stored, main_port)
    elif main_port and main_port.upper() != (origin or "").upper():
        # The VisualGDB project names another port than when the bench port was set.
        if main_port.upper() != stored.upper() and interactive:
            if re.match(r"y(es)?$", ask("VisualGDB project now uses %s, switch to it? [y/N]" % main_port), re.I):
                stored = main_port
            save_port(bench, stored, main_port)
        elif main_port.upper() == stored.upper():
            save_port(bench, stored, main_port)

    if not interactive:
        return stored
    present = bool(stored) and stored.upper() in present_port_names()
    if force_menu or not present:
        if stored and not present:
            output.line("Stored port %s is not present." % stored)
        stored = select_port_menu(stored)
        save_port(bench, stored, main_port)
    return stored
