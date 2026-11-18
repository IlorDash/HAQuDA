"""Removes the bench folder without ever following a junction into a real source folder.

Order: guards (the folder must be a bench and nothing of the repository), the junctions (rmdir
removes only the link), the hard link to the per-user settings (unlink never opens the file), a walk
that refuses any remaining link of any kind, and only then the recursive delete of what is left.
Any doubt ends in a refusal before something is deleted.
"""
import os
import shutil
import stat
import subprocess
import sys

from . import output, winapi
from .build import release_build_lock, take_bench_lock
from .project import BENCH_MARKER, BUILD_LOCK, FW_REL, PROJ, ask, source_candidates

JUNCTIONS = ("sketches", "Libs")


def _norm(path):
    return os.path.normcase(os.path.realpath(path))


def _inside(path, folder):
    return path == folder or path.startswith(folder.rstrip("\\") + "\\")


def _is_reparse(path):
    """True for a junction or symlink. An unreadable entry is an error, never 'not a link': the caller
    stops before deleting anything."""
    try:
        return bool(os.lstat(path).st_file_attributes & 0x400)
    except OSError as e:
        raise RuntimeError("Cannot inspect %s (%s); nothing was deleted." % (path, e))


def find_link(root, skip=()):
    """Path of the first reparse point (junction or symlink, file or folder) at or below root, or
    None. Never descends into one: scandir and lstat do not follow links. The top-level names in
    skip are neither checked nor entered. An entry that cannot be inspected raises."""
    if _is_reparse(root):
        return root
    pending = [root]
    while pending:
        folder = pending.pop()
        try:
            with os.scandir(folder) as entries:
                listed = list(entries)
        except OSError as e:
            raise RuntimeError("Cannot list %s (%s); nothing was deleted." % (folder, e))
        for entry in listed:
            if folder == root and entry.name in skip:
                continue
            if _is_reparse(entry.path):
                return entry.path
            if entry.is_dir(follow_symlinks=False):
                pending.append(entry.path)
    return None


def check_not_repository(bench, main_root):
    """Refuses a bench that is, holds or sits inside the main clone, a worktree or a firmware
    folder. Only main_clone\\.bench itself may sit inside a clone."""
    if os.path.dirname(os.path.abspath(bench)) == os.path.abspath(bench):
        raise RuntimeError("Refusing to remove a drive root.")
    target = _norm(bench)
    default = _norm(os.path.join(main_root, ".bench"))
    for root, _ in source_candidates(main_root):
        for protected in (_norm(root), _norm(os.path.join(root, FW_REL))):
            if _inside(protected, target):
                raise RuntimeError("%s is or holds a source folder (%s); nothing was deleted." % (bench, protected))
            if _inside(target, protected) and target != default:
                raise RuntimeError("%s is inside a source folder (%s) and is not the bench; nothing was deleted." % (bench, protected))
    if _inside(_norm(main_root), target):
        raise RuntimeError("%s holds the main clone; nothing was deleted." % bench)


def _make_writable(func, path, _error):
    """rmtree error handler: a read-only plain file is made writable and removed again. A link or a
    file with several names (the hard-linked settings) is never touched: chmod would change the other name too."""
    if _is_reparse(path) or os.lstat(path).st_nlink > 1:
        raise RuntimeError("Not deleting %s: it is a link or has several names." % path)
    os.chmod(path, stat.S_IWRITE)
    func(path)


def _rmtree(bench):
    # onerror is deprecated since Python 3.12 and removed later; onexc exists from 3.12.
    if sys.version_info >= (3, 12):
        shutil.rmtree(bench, onexc=_make_writable)
    else:
        shutil.rmtree(bench, onerror=_make_writable)


def user_files(bench):
    """Paths of the per-user settings names in the bench."""
    return [os.path.join(bench, n) for n in sorted(os.listdir(bench))
            if n.startswith(PROJ + ".") and n.endswith(".user")]


def remove_bench(bench):
    """Deletes the bench; returns the list of what was removed. Everything that can refuse does so
    before the first deletion; the recursive delete is reached only when no link is left inside."""
    users = user_files(bench)
    for path in users:
        # The bench name is a hard link of the main clone's file, so it has two names. One name means the
        # file is the original (a firmware folder, a copy): unlinking would destroy it.
        if _is_reparse(path) or not os.path.isfile(path) or os.stat(path).st_nlink < 2:
            raise RuntimeError("%s is not a hard link of another file; nothing was deleted." % path)
    found = find_link(bench, skip=JUNCTIONS)
    if found:
        raise RuntimeError("Found a link inside the bench that the script does not know: %s. Nothing was deleted." % found)

    removed = []
    for name in JUNCTIONS:
        link = os.path.join(bench, name)
        if os.path.lexists(link) and _is_reparse(link):
            subprocess.run(["cmd", "/c", "rmdir", link], check=True, stdout=subprocess.DEVNULL)
            removed.append("junction " + link)
    for path in users:
        os.unlink(path)  # removes only this name; the original is untouched
        removed.append("link to the per-user settings " + path)
    # Again over everything: a link that appeared meanwhile, or a junction name that was a real folder.
    found = find_link(bench)
    if found:
        raise RuntimeError("Found a link inside the bench: %s. Removed so far: %s. Nothing else was deleted."
                           % (found, "; ".join(removed) or "nothing"))
    _rmtree(bench)  # reached only when no link is left inside
    removed.append("folder " + bench)
    return removed


def _check_idle(bench):
    # An unreadable record may be half written by a monitor that is starting: counts as running.
    if winapi.record_state(os.path.join(bench, "monitor.pid")) in ("alive", "unreadable"):
        raise RuntimeError("A serial monitor of this bench is running (or starting). Quit it (Ctrl+] in its terminal), then run again.")
    if winapi.lock_held(os.path.join(bench, "flashing.lock")):
        raise RuntimeError("A flash is running. Wait for it to end, then run again.")


def run_clean(bench, confirm, main_root):
    """confirm: 'Yes' or another answer from the VS Code popup, or None to ask in the terminal."""
    bench = os.path.abspath(bench)  # no rstrip: that would turn C:\ into the current folder of drive C
    if not os.path.lexists(bench):
        output.line("Nothing to remove: %s does not exist." % bench)
        return 0
    if _is_reparse(bench):
        raise RuntimeError("%s is a link, not a folder; nothing was deleted." % bench)
    if not os.path.isdir(bench):
        raise RuntimeError("%s is not a folder; nothing was deleted." % bench)
    check_not_repository(bench, main_root)
    if not os.path.isfile(os.path.join(bench, BENCH_MARKER)):
        raise RuntimeError("%s has no %s, so it is not known to be a bench. If it is one from an older version, run any "
                           "HAQuDA task (for example Build or Change COM port) once and try again; otherwise it is not "
                           "a bench. Nothing was deleted." % (bench, BENCH_MARKER))
    _check_idle(bench)

    # The flash lock keeps a flash and a monitor from starting while the bench goes away; the build lock
    # does the same for a build and fails when a build already runs. Both are taken the same exclusive way
    # as by their other users, so a flash or build that starts together with Clean refuses.
    take_bench_lock(bench)
    lock = os.path.join(bench, "flashing.lock")
    try:
        if not winapi.acquire_lock(lock):
            raise RuntimeError("A flash is running. Wait for it to end, then run again.")
        if winapi.record_state(os.path.join(bench, "monitor.pid")) in ("alive", "unreadable"):
            raise RuntimeError("A serial monitor of this bench started meanwhile. Quit it, then run again.")
        if confirm is None:
            answer = ask("Remove %s? The next build is a full one. [y/N]" % bench).lower()
            confirm = "Yes" if answer in ("y", "yes") else "No"
        if confirm != "Yes":
            output.line("Cancelled, nothing was removed.")
            return 0
        # The guard is held while the folder is deleted: a Build, Flash or Port that starts now waits for it
        # (up to 5 s, then refuses) instead of creating files in a folder that is going away. It then
        # finds no bench and creates a fresh one.
        with winapi.exclusive(os.path.join(bench, BUILD_LOCK)):
            for item in remove_bench(bench):
                output.line("Removed " + item)
        return 0
    finally:
        winapi.release_lock(lock)
        release_build_lock(bench)  # the folder may be gone already: nothing to do then
