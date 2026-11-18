"""The bench folder: file helpers, source folder menu, bench project, per-user settings link, junctions."""
import os
import re
import subprocess

from . import output

FW_REL = "Software\\Software_ver_2\\HAQuDA_Esp32"
PROJ = "HAQuDA_Esp32.vgdbproj"
PLATFORM = "ESP32_Dev_Module"
REPARSE_POINT = 0x400
COLORS_MACRO = "CONFIG_ARDUHAL_LOG_COLORS=1"


def read_latin1(path):
    """Latin-1 maps bytes to chars one to one, so everything but the replaced text stays identical."""
    with open(path, "rb") as f:
        return f.read().decode("latin-1")


def first_line(path):
    """First line of a small text file, stripped; None when missing or empty."""
    try:
        with open(path, encoding="utf-8-sig", errors="replace") as f:
            return f.readline().strip() or None
    except OSError:
        return None


def write_text(path, text):
    with open(path, "w", encoding="utf-8") as f:
        f.write(text + "\n")


def ask(prompt):
    """A line from the terminal; a closed input counts as an empty answer."""
    try:
        return input(prompt + ": ").strip()
    except EOFError:
        return ""


def same_path(a, b):
    if not a or not b:
        return False
    return os.path.normcase(os.path.abspath(a)).rstrip("\\") == os.path.normcase(os.path.abspath(b)).rstrip("\\")


def main_clone_root(script_dir):
    """The folder that owns the common .git directory, also when this script runs from a worktree."""
    try:
        done = subprocess.run(["git", "-C", script_dir, "rev-parse", "--git-common-dir"],
                              stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        git_dir = done.stdout.decode("utf-8", "replace").strip() if done.returncode == 0 else ""
    except OSError:
        git_dir = ""
    if not git_dir:
        raise RuntimeError("Cannot find the git directory of the repository that holds %s (is git on PATH?)" % script_dir)
    # A relative answer (".git" in the main clone) is relative to the directory given to -C.
    git_dir = os.path.abspath(os.path.join(script_dir, git_dir))
    if os.path.basename(git_dir) != ".git":
        raise RuntimeError("Unsupported repository layout: the common git directory is %s, expected a folder named .git" % git_dir)
    return os.path.dirname(git_dir)


# ---- junctions ----------------------------------------------------------------------------------

def _is_link(path):
    # isdir()/exists() say false for a junction whose target is gone, so read the attributes.
    try:
        return bool(os.lstat(path).st_file_attributes & REPARSE_POINT)
    except OSError:
        return False


def set_junction(link, target):
    """Points link at target. A wrong link is removed with rmdir, which deletes only the link and
    never what it points at; nothing here deletes recursively."""
    target = os.path.abspath(target).rstrip("\\")
    if not os.path.isdir(target):
        raise RuntimeError("Junction target does not exist: " + target)
    if _is_link(link):
        try:
            current = os.readlink(link)  # the substitution name, usually with a \\?\ prefix
        except OSError:
            current = ""
        if current.replace("\\\\?\\", "", 1).rstrip("\\").lower() == target.lower():
            return
        subprocess.run(["cmd", "/c", "rmdir", link], check=True, stdout=subprocess.DEVNULL)
    elif os.path.lexists(link):
        raise RuntimeError("%s exists and is not a junction; remove it by hand" % link)
    done = subprocess.run(["cmd", "/c", "mklink", "/J", link, target], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if done.returncode != 0:
        raise RuntimeError("Cannot create the junction %s: %s" % (link, done.stdout.decode("oem", "replace").strip()))


# ---- source folder menu -------------------------------------------------------------------------

def _branch_label(git_dir):
    head = first_line(os.path.join(git_dir, "HEAD")) or ""
    m = re.match(r"ref:\s*refs/heads/(.+)$", head)
    if m:
        return m.group(1).strip()
    return "detached " + head[:7] if re.match(r"[0-9a-f]{7,}", head) else "?"


def source_candidates(main_root):
    """The main clone plus every worktree that holds the firmware sketches. Reads the worktree
    registry in .git directly instead of running git."""
    found = [(main_root, _branch_label(os.path.join(main_root, ".git")))]
    registry = os.path.join(main_root, ".git", "worktrees")
    for name in sorted(os.listdir(registry)) if os.path.isdir(registry) else []:
        entry = os.path.join(registry, name)
        git_file = first_line(os.path.join(entry, "gitdir"))  # "<worktree root>/.git"
        if git_file:
            root = os.path.dirname(os.path.normpath(git_file))
            if os.path.isdir(os.path.join(root, FW_REL, "sketches")):
                found.append((root, _branch_label(entry)))
    return found


def source_choices(main_root, bench):
    """(candidates, 1-based index of the last used source or 0 when it is not among them)."""
    cands = source_candidates(main_root)
    last = first_line(os.path.join(bench, "last-source.txt"))
    index = 0
    for i, (root, _) in enumerate(cands, 1):
        if same_path(root, last):
            index = i
    return cands, index


def source_list(main_root, bench):
    """Sources for the VS Code picker, last used first. Read-only: creates nothing in the bench."""
    cands, last = source_choices(main_root, bench)
    items = [{"label": branch, "path": root, "last": i == last} for i, (root, branch) in enumerate(cands, 1)]
    return sorted(items, key=lambda item: not item["last"])  # stable: the rest keeps the menu order


def select_source(main_root, bench):
    cands, last = source_choices(main_root, bench)
    default = last or 1
    output.line("Folders with firmware sources (* = last used):")
    for i, (root, branch) in enumerate(cands, 1):
        output.line("%s [%d] %-26s %s" % ("*" if i == default else " ", i, branch, root))
    answer = ask("Number (Enter = %d)" % default)
    if not answer:
        return cands[default - 1][0]
    if not (answer.isdigit() and 1 <= int(answer) <= len(cands)):
        raise RuntimeError("Not a valid choice: " + answer)
    return cands[int(answer) - 1][0]


# ---- bench project ------------------------------------------------------------------------------

def update_bench_project(main_fw, bench, port):
    """Copy of the main project with the output folder moved into the bench and the port patched in."""
    src = os.path.join(main_fw, PROJ)
    text = read_latin1(src)
    old = "<OutputSubdirectory>Output\\$(PlatformName.defuse)"
    if old not in text:
        raise RuntimeError("OutputSubdirectory not found in %s; the script must be updated" % src)
    text = text.replace(old, "<OutputSubdirectory>build\\$(PlatformName.defuse)")
    # Covers main-clone projects older than the change that put this macro (coloured log_e/w/i/d/v) into the
    # project. Added after the existing macros; the secret values stay in the per-user file.
    if COLORS_MACRO not in text:
        text, found = re.subn(r"(<PreprocessorMacros>\s*<Common>.*?)(\r?\n[ \t]*)</Common>",
                              lambda m: "%s%s  <string>%s</string>%s</Common>" % (m.group(1), m.group(2), COLORS_MACRO, m.group(2)),
                              text, count=1, flags=re.S)
        if not found:
            raise RuntimeError("PreprocessorMacros/Common not found in %s; the script must be updated" % src)
    if port:
        # A project without the port field is fine: the port is asked for when it is needed.
        for tag in ("COMPort", "ComPortName"):
            text = re.sub("<%s>[^<]*</%s>" % (tag, tag), lambda _m, t=tag: "<%s>%s</%s>" % (t, port, t), text)
    data, dst = text.encode("latin-1"), os.path.join(bench, PROJ)
    try:
        with open(dst, "rb") as f:
            if f.read() == data:
                return  # written only on change, so VisualGDB is not disturbed by a touched file
    except OSError:
        pass
    with open(dst, "wb") as f:
        f.write(data)


def link_user_file(main_fw, bench):
    """The per-user settings file holds the secret build flags: hard-linked, never read, copied or printed."""
    name = "%s.%s." % (PROJ, os.environ.get("USERNAME", "")) + "user"
    src, dst = os.path.join(main_fw, name), os.path.join(bench, name)
    if not os.path.exists(src):
        raise RuntimeError("The per-user VisualGDB settings file of the main clone is missing (%s in %s). Open the project "
                           "once in Visual Studio and set the build flags, then run again." % (name, main_fw))
    if same_path(src, dst):
        raise RuntimeError("The bench folder must not be the firmware folder of the main clone")
    # Recreated on every run: cheap, and it repairs a link that an editor broke by saving the original
    # through write+rename. Deleting the bench name never touches the original.
    if os.path.lexists(dst):
        os.unlink(dst)
    try:
        os.link(src, dst)
    except OSError as e:
        raise RuntimeError("Cannot hard-link the per-user settings file into the bench. A hard link needs the bench and the "
                           "main clone on the same volume; choose --bench-root on the same drive as %s. (%s)" % (main_fw, e))


def init_bench_source(source_root, main_root, bench):
    main_fw = os.path.join(main_root, FW_REL)
    sketches = os.path.join(source_root, FW_REL, "sketches")
    if not os.path.isdir(sketches):
        raise RuntimeError("No sketches folder at " + sketches)
    link_user_file(main_fw, bench)
    set_junction(os.path.join(bench, "Libs"), os.path.join(main_fw, "Libs"))
    set_junction(os.path.join(bench, "sketches"), sketches)
    write_text(os.path.join(bench, "last-source.txt"), source_root)
