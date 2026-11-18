"""Windows calls through ctypes: console modes, the process records that hand the COM port over
between the flash step and the monitor, and a kill-on-close job object."""
import contextlib
import ctypes
import hashlib
import os
import time
from ctypes import byref, c_int, c_uint32, c_ulonglong, c_void_p

k32 = ctypes.WinDLL("kernel32", use_last_error=True)
k32.GetStdHandle.restype = c_void_p
k32.GetStdHandle.argtypes = [c_uint32]
k32.GetConsoleMode.argtypes = [c_void_p, ctypes.POINTER(c_uint32)]
k32.SetConsoleMode.argtypes = [c_void_p, c_uint32]
k32.FlushConsoleInputBuffer.argtypes = [c_void_p]
k32.OpenProcess.restype = c_void_p
k32.OpenProcess.argtypes = [c_uint32, c_int, c_uint32]
k32.GetProcessTimes.argtypes = [c_void_p] + [ctypes.POINTER(c_ulonglong)] * 4
k32.CloseHandle.argtypes = [c_void_p]
k32.CreateJobObjectW.restype = c_void_p
k32.CreateJobObjectW.argtypes = [c_void_p, ctypes.c_wchar_p]
k32.SetInformationJobObject.argtypes = [c_void_p, c_int, c_void_p, c_uint32]
k32.AssignProcessToJobObject.argtypes = [c_void_p, c_void_p]
k32.CreateMutexW.restype = c_void_p
k32.CreateMutexW.argtypes = [c_void_p, c_int, ctypes.c_wchar_p]
k32.WaitForSingleObject.restype = c_uint32
k32.WaitForSingleObject.argtypes = [c_void_p, c_uint32]
k32.ReleaseMutex.argtypes = [c_void_p]

STD_INPUT, STD_OUTPUT = 0xFFFFFFF6, 0xFFFFFFF5
ENABLE_VT = 4
# A record that cannot be parsed yet may just be half written; only a young one counts as held.
YOUNG_LOCK_S = 5


def enable_vt():
    """Turns on escape-sequence processing of the console output; False when it cannot be."""
    handle, mode = k32.GetStdHandle(STD_OUTPUT), c_uint32()
    if not handle or not k32.GetConsoleMode(handle, byref(mode)):
        return False
    return bool(mode.value & ENABLE_VT) or bool(k32.SetConsoleMode(handle, mode.value | ENABLE_VT))


def console_modes():
    """Console input and output modes now: esp-idf-monitor changes them and a killed one cannot undo it."""
    modes = []
    for std in (STD_INPUT, STD_OUTPUT):
        handle, mode = k32.GetStdHandle(std), c_uint32()
        if handle and k32.GetConsoleMode(handle, byref(mode)):
            modes.append((handle, mode.value))
    return modes


def restore_console(modes):
    for handle, mode in modes:
        k32.SetConsoleMode(handle, mode)


def flush_console_input():
    """Drops keys typed while the monitor was paused, so they cannot quit the restarted one."""
    k32.FlushConsoleInputBuffer(k32.GetStdHandle(STD_INPUT))


class _BasicLimits(ctypes.Structure):
    _fields_ = [("PerProcessUserTimeLimit", ctypes.c_int64), ("PerJobUserTimeLimit", ctypes.c_int64),
                ("LimitFlags", c_uint32), ("MinimumWorkingSetSize", ctypes.c_size_t),
                ("MaximumWorkingSetSize", ctypes.c_size_t), ("ActiveProcessLimit", c_uint32),
                ("Affinity", ctypes.c_size_t), ("PriorityClass", c_uint32), ("SchedulingClass", c_uint32)]


class _ExtendedLimits(ctypes.Structure):
    _fields_ = [("Basic", _BasicLimits), ("IoInfo", c_ulonglong * 6), ("ProcessMemoryLimit", ctypes.c_size_t),
                ("JobMemoryLimit", ctypes.c_size_t), ("PeakProcessMemoryUsed", ctypes.c_size_t),
                ("PeakJobMemoryUsed", ctypes.c_size_t)]


def kill_on_close_job():
    """A job object that kills its processes when its last handle closes, also when this process is
    killed: the monitor must not outlive its wrapper and keep the port. None on failure."""
    job = k32.CreateJobObjectW(None, None)
    if not job:
        return None
    limits = _ExtendedLimits()
    limits.Basic.LimitFlags = 0x2000  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    # 9 = JobObjectExtendedLimitInformation
    if not k32.SetInformationJobObject(job, 9, byref(limits), ctypes.sizeof(limits)):
        return None
    return job


_job = None


def adopt(popen):
    """Puts a child (and what it starts later) into one kill-on-close job: when this process ends,
    however it ends, no child outlives it to hold the port or write into the bench."""
    global _job
    if _job is None:
        _job = kill_on_close_job() or False
    if _job:
        k32.AssignProcessToJobObject(_job, int(popen._handle))


# ---- lock and pid records: "PID\nprocess creation FILETIME\n" ---------------------------------

def process_start(pid):
    """Creation FILETIME of a running process; None when there is none; 0 when it exists but its
    times cannot be read (a record with 0 then matches by PID alone)."""
    handle = k32.OpenProcess(0x1000, False, pid)  # PROCESS_QUERY_LIMITED_INFORMATION
    if not handle:
        return 0 if ctypes.get_last_error() == 5 else None  # access denied: it exists
    try:
        times = [c_ulonglong() for _ in range(4)]
        if not k32.GetProcessTimes(handle, *[byref(t) for t in times]):
            return 0
        # An exit time means the process ended and only a stray handle keeps its entry.
        return None if times[1].value else times[0].value
    finally:
        k32.CloseHandle(handle)


def write_record(path):
    with open(path, "w", encoding="ascii") as f:
        f.write("%d\n%d\n" % (os.getpid(), process_start(os.getpid()) or 0))


def record_pid(path):
    """First token of a record as text, None when it cannot be read."""
    try:
        with open(path, encoding="ascii") as f:
            return (f.read().split() or [None])[0]
    except (OSError, UnicodeDecodeError):
        return None


def record_state(path):
    """'none' (no file), 'unreadable' (empty or half written), 'stale' (process gone) or 'alive'."""
    try:
        with open(path, encoding="ascii") as f:
            fields = f.read().split()
    except FileNotFoundError:
        return "none"
    except (OSError, UnicodeDecodeError):
        return "unreadable"
    try:
        pid, start = int(fields[0]), int(fields[1])
    except (IndexError, ValueError):
        return "unreadable"
    actual = process_start(pid)
    if actual is None:
        return "stale"
    return "alive" if 0 in (start, actual) or actual == start else "stale"  # 0 = unknown: the PID alone counts


def lock_held(path):
    """A live owner holds the lock; a young unreadable file counts as held, a stale one never does."""
    state = record_state(path)
    if state == "unreadable":
        try:
            return time.time() - os.stat(path).st_mtime < YOUNG_LOCK_S
        except OSError:
            return False
    return state == "alive"


def held_by_other(path):
    """True when a live process other than this one holds the lock."""
    return lock_held(path) and record_pid(path) != str(os.getpid())


@contextlib.contextmanager
def exclusive(path):
    """A named kernel mutex for the lock file `path`: what runs inside is alone with respect to every
    other holder of the same name. acquire_lock uses it for check, removal and creation of the record;
    Clean holds it while it deletes the bench, so no starter can create files in a folder that is going
    away. The system frees it when its owner dies, so it never goes stale. Raises RuntimeError when the
    mutex cannot be created, or is not free within 5 s."""
    name = "Local\\HAQuDA-bench-" + hashlib.sha1(os.path.normcase(os.path.abspath(path)).encode("utf-8")).hexdigest()
    handle = k32.CreateMutexW(None, False, name)
    if not handle:
        raise RuntimeError("Cannot create the Windows mutex that guards the bench (error %d)." % ctypes.get_last_error())
    if k32.WaitForSingleObject(handle, 5000) not in (0, 0x80):  # WAIT_OBJECT_0, or WAIT_ABANDONED: the owner died
        k32.CloseHandle(handle)
        raise RuntimeError("The bench is busy: another HAQuDA task is starting or a Remove bench is deleting it. "
                           "Wait for it to end, then run again.")
    try:
        yield
    finally:
        k32.ReleaseMutex(handle)
        k32.CloseHandle(handle)


def acquire_lock(path):
    """Takes the lock; False when a live process holds it. A record of a dead process is replaced.
    Check, removal and creation run under exclusive(), so two starters that see the same stale record
    cannot both end up holding the lock. The folder of `path` must exist."""
    with exclusive(path):
        for _ in range(10):
            if lock_held(path):
                return False
            remove_quiet(path)
            try:
                with open(path, "x", encoding="ascii") as f:
                    f.write("%d\n%d\n" % (os.getpid(), process_start(os.getpid()) or 0))
                return True
            except FileExistsError:  # the removal did not go through (a reader had the file open)
                time.sleep(0.05)
        return False


def release_lock(path):
    """Removes the lock when this process owns it. Gives up quietly: a record left behind names a
    process that is gone by then and reads as stale."""
    for _ in range(5):
        pid = record_pid(path)
        if pid == str(os.getpid()):
            remove_quiet(path)
            return
        if pid is not None or not os.path.exists(path):
            return  # not ours, or already gone
        time.sleep(0.05)  # unreadable for a moment


def _unlink_retry(path):
    """Deletes a file; Windows refuses while another process has it open (Python opens without delete
    sharing), so a few retries. A missing file is fine; the last other error is raised."""
    for attempt in range(5):
        try:
            os.unlink(path)
            return
        except FileNotFoundError:
            return
        except OSError:
            if attempt == 4:
                raise
            time.sleep(0.05)


def remove(path):
    """Deletes a file after a few retries; raises OSError when it cannot. For files whose absence the
    caller relies on (the monitor hand-over acknowledgement, the monitor's own records)."""
    _unlink_retry(path)


def remove_quiet(path):
    """Like remove, but gives up without an error: for lock records, where a leftover reads as stale."""
    try:
        _unlink_retry(path)
    except OSError:
        pass
