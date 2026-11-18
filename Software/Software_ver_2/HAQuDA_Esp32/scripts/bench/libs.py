"""Warns before a build when the chosen folder differs from the main clone in what the bench takes
from the main clone: Libs and the VisualGDB project."""
import os
import re
import subprocess

from . import output
from .project import COLORS_MACRO, FW_REL, PROJ, read_latin1, same_path

LIBS = FW_REL.replace("\\", "/") + "/Libs"


def _build_relevant_project(path):
    """Project text without what does not change the build: the OtherFiles list (header names for
    the IDE only) and the COM port (patched per bench)."""
    if not os.path.isfile(path):
        return None
    text = re.sub(r"<OtherFiles>.*?</OtherFiles>", "", read_latin1(path).replace("\r\n", "\n"), flags=re.S)
    # The bench adds this macro itself when the main project lacks it, so it is no difference worth a warning.
    text = re.sub(r"[ \t]*<string>%s</string>\n" % re.escape(COLORS_MACRO), "", text)
    return re.sub(r"<COMPort>[^<]*</COMPort>|<ComPortName>[^<]*</ComPortName>", "", text)


def _git(root, *args):
    """Output of one git call in root; raises when git fails. --no-optional-locks keeps "status" from
    taking the index lock, which would fight with the git view of an open editor."""
    done = subprocess.run(["git", "--no-optional-locks", "-C", root] + list(args),
                          stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    if done.returncode != 0:
        raise RuntimeError("git %s failed in %s" % (args[0], root))
    return done.stdout.decode("utf-8", "replace")


def _libs_state(root):
    """What git knows about Libs in one checkout: the tree hash at HEAD, the changed entries of the
    working tree, and per submodule its commit and whether it is checked out ("git submodule
    status" marks one that is not with a leading '-')."""
    subs = {}
    for text in _git(root, "submodule", "status", "--", LIBS).splitlines():
        m = re.match(r"([ +\-U])([0-9a-f]{40,64}) (\S+)", text)
        if m:
            subs[m.group(3).split("/")[-1]] = (m.group(2), m.group(1) != "-")
    dirty = set(re.findall(r'Libs/([^/\s"]+)', _git(root, "status", "--porcelain", "--", LIBS)))
    return _git(root, "rev-parse", "HEAD:" + LIBS).strip(), dirty, subs


def _libs_difference(source_root, main_root):
    """(not checked out, different, any) names for the Libs of two checkouts."""
    s_tree, s_dirty, s_subs = _libs_state(source_root)
    m_tree, m_dirty, m_subs = _libs_state(main_root)
    not_out, different = [], set(s_dirty) | set(m_dirty)
    for name in sorted(set(s_subs) | set(m_subs)):
        s, m = s_subs.get(name), m_subs.get(name)
        if s and m and not s[1] and m[1]:
            not_out.append(name)
        elif s != m:
            different.add(name)
    return not_out, sorted(different), s_tree != m_tree or bool(different) or bool(not_out)


def check_source_matches_main(source_root, main_root):
    """Warns, never stops: the bench builds the sketches of the chosen folder with the main clone's
    Libs and project, so a difference there silently gives a different firmware than that folder would."""
    if same_path(source_root, main_root):
        return
    src = _build_relevant_project(os.path.join(source_root, FW_REL, PROJ))
    if src is None:
        output.warn("%s has no %s; the bench builds with the project of the main clone." % (source_root, PROJ))
    elif src != _build_relevant_project(os.path.join(main_root, FW_REL, PROJ)):
        output.warn("The VisualGDB project of %s differs from the main clone's in a build-relevant way (board settings, "
                    "flags or paths; a changed OtherFiles list does not count). The bench builds with the main clone's "
                    "project." % source_root)
    try:
        with output.spinner("Checking Libs", plain_line=False):
            not_out, different, any_diff = _libs_difference(source_root, main_root)
    except (RuntimeError, OSError):
        output.warn("could not compare Libs")
        return
    for name in not_out:
        output.warn("submodule %s is not checked out in %s (git submodule update --init); the bench uses the main "
                    "clone's copy anyway." % (name, source_root))
    if different:
        output.warn("Libs of %s differ from the main clone's in: %s. The bench builds with the main clone's Libs."
                    % (source_root, ", ".join(different)))
    elif any_diff and not not_out:
        output.warn("Libs of %s differ from the main clone's. The bench builds with the main clone's Libs." % source_root)
