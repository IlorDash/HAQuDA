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

Without --source a numbered menu lists the main clone and every worktree. The COM port lives in
<bench>\\port and is patched into the bench copy of the project only; no tracked file is changed.

Monitor and flash share the port without help from the user: the flash step writes
<bench>\\flashing.lock (its PID and start time), the monitor sees it, stops esp-idf-monitor, and
starts it again when the lock is gone. A lock whose process is gone is ignored. Only one monitor
runs per bench (<bench>\\monitor.pid). The monitor needs esp-idf-monitor:
py -3 -m pip install esp-idf-monitor

Python 3.8+, standard library only: py -3 haquda-bench.py --action Build. Never deletes anything
recursively: the junctions point at real source folders.
"""
import sys

sys.dont_write_bytecode = True  # no __pycache__ in the repository

import argparse
import os
import re
import time

from bench import build, flash, libs, monitor, output, port, project

ACTIONS = ("Build", "Flash", "BuildFlash", "Monitor", "Port")
CONFIGS = ("Debug", "Release")


def one_of(options):
    """argparse type: case-insensitive choice that returns the canonical spelling."""
    def parse(text):
        for option in options:
            if option.lower() == text.lower():
                return option
        raise argparse.ArgumentTypeError("%r is not one of %s" % (text, ", ".join(options)))
    return parse


def parse_args():
    p = argparse.ArgumentParser(description="Builds, flashes and monitors the firmware through a shared bench folder.")
    p.add_argument("--source", help="root of the clone or worktree whose sketches folder is built; optional, a menu asks. "
                                    "Flash, Monitor and Port ignore it: they work on the last bench build")
    p.add_argument("--bench-root", help="default: <main clone root>\\.bench")
    p.add_argument("--config", type=one_of(CONFIGS), default="Debug", metavar="{Debug,Release}")
    p.add_argument("--action", type=one_of(ACTIONS), default="Build", metavar="{Build,Flash,BuildFlash,Monitor,Port}")
    p.add_argument("--port", type=lambda t: t.upper() if re.fullmatch(r"COM\d+", t, re.I) else p.error("--port must look like COM3"),
                   help="COM port to use and remember; without it the stored port is used")
    p.add_argument("--dry-run", action="store_true", help="print the esptool and esp-idf-monitor command lines instead of running them")
    p.add_argument("--no-timestamps", action="store_true", help="no host time at the start of each output line")
    p.add_argument("--no-color", action="store_true", help="no colours and no dim time (also when the output is redirected)")
    p.add_argument("--trace", action="store_true", help="print how long each preparation step took")
    return p.parse_args()


def main(args):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    main_root = project.main_clone_root(script_dir)
    main_fw = os.path.join(main_root, project.FW_REL)
    # Full path before any use: child processes resolve a relative path against their own directory.
    bench = os.path.abspath(args.bench_root or os.path.join(main_root, ".bench")).rstrip("\\")
    need_build, need_flash = args.action in ("Build", "BuildFlash"), args.action in ("Flash", "BuildFlash")

    os.makedirs(os.path.join(bench, "build"), exist_ok=True)
    output.init(args.no_timestamps, args.no_color, args.trace)
    output.trace("start, output style")

    if args.action == "Monitor":
        # Starts together with the build in the next terminal: never asks, never touches the project.
        return monitor.run_monitor(main_fw, bench, port.resolve_port(main_fw, bench, args.port, False, False), args.config, args.dry_run)

    source_root = None
    if need_build:
        if args.source:
            source_root = os.path.abspath(args.source).rstrip("\\")
            if not os.path.isdir(source_root):
                raise RuntimeError("Cannot find path '%s' because it does not exist." % source_root)
        else:
            source_root = project.select_source(main_root, bench)
    output.trace("source menu")

    # Build only patches the stored port; every other action may ask about it.
    com_port = port.resolve_port(main_fw, bench, args.port, args.action != "Build", args.action == "Port")
    output.trace("resolve_port")
    with output.spinner("Preparing the bench project", plain_line=False):  # local files only, normally instant
        project.update_bench_project(main_fw, bench, com_port)
    output.trace("update_bench_project")

    if args.action == "Port":
        output.line("COM port: %s (stored in %s)" % (com_port, os.path.join(bench, "port")))
        return 0

    build_start = None
    if need_build:
        libs.check_source_matches_main(source_root, main_root)
        output.trace("check_source_matches_main")
        with output.spinner("Preparing the bench folders", plain_line=False):
            project.init_bench_source(source_root, main_root, bench)
        output.trace("init_bench_source")
        build_start = time.time()
        if need_flash:
            output.line("If your lamp has no automatic reset, put it into the bootloader mode while it builds; "
                        "the flash starts right after the build.")
        code = build.run_build(bench, args.config)
        if code != 0:
            return code
    if need_flash:
        return flash.run_flash(bench, args.config, com_port, build_start, args.dry_run)
    return 0


if __name__ == "__main__":
    arguments = parse_args()
    try:
        sys.exit(main(arguments))
    except KeyboardInterrupt:
        sys.exit(130)
    except Exception as e:  # any failure ends with one red line and exit code 1
        output.line("ERROR: %s" % e, "31")
        sys.exit(1)
