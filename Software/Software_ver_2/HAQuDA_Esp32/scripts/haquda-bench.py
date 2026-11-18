"""Builds, flashes and monitors the firmware of any clone or worktree through one shared "bench" folder.

Why a bench: arduino-builder records absolute paths in its build cache. A fixed project path and a
fixed output folder keep the cache valid, so switching the source folder recompiles only the sketch
files that differ; the core and Libs stay cached. Only "sketches" comes from the chosen source
folder, everything else comes from the main clone.

Actions:
  Build        build the chosen source folder, with live progress
  Flash        write the last bench build to the lamp over UART (esptool)
  BuildFlash   Build, then Flash
  Monitor      serial monitor (esp-idf-monitor), quit with Ctrl+]
  Port         choose the COM port again
  Sources      JSON list of the source folders for the VS Code picker, last used first
  Ports        JSON list of the COM ports for the VS Code picker, stored port first
  InstallPicker  pack scripts/vscode-picker into a .vsix and install it with "code"
  Clean        remove the bench safely (--confirm Yes, or a y/N question); the next build is a full one

Without --source a numbered menu lists the main clone and every worktree. The COM port lives in
<bench>\\port and is patched into the bench copy of the project only; no tracked file is changed.

Monitor and flash share the port without help from the user: the flash step writes
<bench>\\flashing.lock (its PID and start time), the monitor sees it, stops esp-idf-monitor, and
starts it again when the lock is gone. A lock whose process is gone is ignored. Only one monitor
runs per bench (<bench>\\monitor.pid). The monitor needs esp-idf-monitor:
py -3 -m pip install esp-idf-monitor

Builds never run at the same time: Build, Flash, Port and Clean hold <bench>\\building.lock while they
write in the bench (a build for its whole run). Another one of them refuses while it is held, and a
Build also refuses while a flash runs. A lock whose process is gone is ignored.

Python 3.8+, standard library only: py -3 haquda-bench.py --action Build. Never deletes anything
recursively: the junctions point at real source folders.
"""
import sys

sys.dont_write_bytecode = True  # no __pycache__ in the repository

import argparse
import json
import os
import re
import time

from bench import build, clean, flash, libs, monitor, output, picker, port, project, winapi

ACTIONS = ("Build", "Flash", "BuildFlash", "Monitor", "Port", "Sources", "Ports", "InstallPicker", "Clean")
PICK_OPTIONS = ("--source", "--port", "--confirm")  # filled by VS Code popups


def one_of(options):
    """argparse type: case-insensitive choice that returns the canonical spelling."""
    def parse(text):
        for option in options:
            if option.lower() == text.lower():
                return option
        raise argparse.ArgumentTypeError("%r is not one of %s" % (text, ", ".join(options)))
    return parse


def cancelled_pick(argv):
    """True when a value that a VS Code popup should have supplied is empty or still the literal
    ${input:...} text: the user pressed Esc. Looks at the raw arguments, because argparse would
    reject such a value (or the missing one) before any code could run."""
    for i, arg in enumerate(argv):
        for option in PICK_OPTIONS:
            if arg == option and i + 1 < len(argv):
                value = argv[i + 1]
            elif arg.startswith(option + "="):
                value = arg[len(option) + 1:]
            else:
                continue
            if value == "" or value.startswith("${input:"):
                return True
    return False


def parse_args():
    p = argparse.ArgumentParser(description="Builds, flashes and monitors the firmware through a shared bench folder.")
    p.add_argument("--source", help="root of the clone or worktree whose sketches folder is built; optional, a menu asks. "
                                    "Flash, Monitor and Port ignore it: they work on the last bench build")
    p.add_argument("--action", type=one_of(ACTIONS), default="Build", metavar="{%s}" % ",".join(ACTIONS))
    p.add_argument("--port", type=lambda t: t.upper() if re.fullmatch(r"COM\d+", t, re.I) else p.error("--port must look like COM3"),
                   help="COM port to use and remember; without it the stored port is used")
    p.add_argument("--confirm", help="Clean only: 'Yes' removes the bench, any other value does nothing; without it the script asks y/N")
    p.add_argument("--dry-run", action="store_true", help="print the esptool and esp-idf-monitor command lines instead of running them")
    return p.parse_args()


def main(args):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    main_root = project.main_clone_root(script_dir)
    main_fw = os.path.join(main_root, project.FW_REL)
    # Full path before any use: child processes resolve a relative path against their own directory.
    bench = os.path.abspath(os.path.join(main_root, ".bench"))
    need_build, need_flash = args.action in ("Build", "BuildFlash"), args.action in ("Flash", "BuildFlash")

    if args.action in ("Sources", "Ports"):
        # Machine output for the VS Code picker: no timestamps, no prompts, nothing created in the bench.
        # Errors go to stderr, which the picker shows to the user.
        try:
            items = project.source_list(main_root, bench) if args.action == "Sources" else port.port_list(main_fw, bench)
        except Exception as e:
            sys.stderr.write("%s\n" % e)
            return 1
        sys.stdout.write(json.dumps(items) + "\n")  # ASCII-only JSON: safe on any console code page
        return 0

    if args.action == "Clean":
        # Removing a bench must not create one first.
        output.init()
        return clean.run_clean(bench, args.confirm, main_root)

    output.init()

    # Monitor and InstallPicker run next to a build and take no bench lock; they only need the folder.
    # The folder is created under the guard mutex that Clean holds while it deletes the bench (a short hold).
    if args.action == "InstallPicker":
        with winapi.exclusive(os.path.join(bench, project.BUILD_LOCK)):
            project.ensure_bench(bench)
        return picker.install(bench)

    if args.action == "Monitor":
        # Starts together with the build in the next terminal: never asks, never touches the project.
        choice = port.resolve_port(main_fw, bench, args.port, False, False)
        with winapi.exclusive(os.path.join(bench, project.BUILD_LOCK)):
            project.ensure_bench(bench)
            port.store_port(bench, choice)
        return monitor.run_monitor(main_fw, bench, choice.port, project.CONFIG, args.dry_run)

    source_root = None
    if need_build:
        if args.source:
            source_root = os.path.abspath(args.source).rstrip("\\")
            if not os.path.isdir(source_root):
                raise RuntimeError("Cannot find path '%s' because it does not exist." % source_root)
        else:
            source_root = project.select_source(main_root, bench)

    # Build only patches the stored port; every other action may ask about it. The questions are asked
    # before the bench lock, and nothing is written yet (the bench may not exist).
    choice = port.resolve_port(main_fw, bench, args.port, args.action != "Build", args.action == "Port")
    com_port = choice.port

    # The bench lock is taken before the first shared file is written (the port file, the bench project, the
    # sketches junction, the builder output) by every action that writes there, and released right after the
    # build or the project refresh, before the flash. So builds never overlap, and a Flash or Port cannot
    # rewrite the project under a running build. The bench folder itself is created under the lock.
    build.acquire_build_lock(bench)
    build_start = None
    try:
        if need_build:
            build.refuse_while_flashing(bench)  # a build must not overwrite the images esptool reads
        port.store_port(bench, choice)
        with output.spinner("Preparing the bench project", plain_line=False):  # local files only, normally instant
            project.update_bench_project(main_fw, bench, com_port)

        if args.action == "Port":
            output.line("COM port: %s (stored in %s)" % (com_port, os.path.join(bench, "port")))
            return 0

        if need_build:
            libs.check_source_matches_main(source_root, main_root)
            with output.spinner("Preparing the bench folders", plain_line=False):
                project.init_bench_source(source_root, main_root, bench)
            build_start = time.time()
            if need_flash:
                output.line("If your lamp has no automatic reset, put it into the bootloader mode while it builds; "
                            "the flash starts right after the build.")
            code = build.run_build(bench, project.CONFIG)
            if code != 0:
                return code
            project.remember_source(bench, source_root)
    finally:
        build.release_build_lock(bench)
    if need_flash:
        return flash.run_flash(bench, project.CONFIG, com_port, build_start, args.dry_run)
    return 0


if __name__ == "__main__":
    # Esc in a popup is a clean cancel, whatever VS Code passes on; nothing is touched.
    if cancelled_pick(sys.argv[1:]):
        sys.stdout.write("Cancelled: nothing picked.\n")
        sys.exit(0)
    arguments = parse_args()
    try:
        sys.exit(main(arguments))
    except KeyboardInterrupt:
        sys.exit(130)
    except Exception as e:  # any failure ends with one red line and exit code 1
        output.line("ERROR: %s" % e, "31")
        sys.exit(1)
