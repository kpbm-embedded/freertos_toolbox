"""List host ELF binaries under a directory that load shared libraries from the OS.

Usage: python3 -I libcheck.py <dir> [<dir> ...]
A library is fine if it resolves inside the mcu workspace or is part of glibc.
libstdc++/libgcc_s are reported as "gcc-runtime" (OS gcc-libs) but not counted as failures.
Exit 1 if any binary needs another OS library or has an unresolved one.
"""
import os
import subprocess
import sys

GLIBC = {
    "libc.so.6", "libm.so.6", "libdl.so.2", "libpthread.so.0", "librt.so.1", "libutil.so.1",
    "libresolv.so.2", "libanl.so.1", "libmvec.so.1", "libnsl.so.1", "libcrypt.so.1",
    "ld-linux-x86-64.so.2", "linux-vdso.so.1",
}
GCC_RT = {"libstdc++.so.6", "libgcc_s.so.1", "libatomic.so.1", "libgomp.so.1"}
# Explicit, narrow allowlist: reported as "optional, unused" instead of failing. Keep it short.
#  - esp-idf/.../*-esp-elf-gdb-3.N for N != 12: the IDF venv is Python 3.12, so only the -3.12 gdb is used.
#    (3.8/3.13 resolve to host/python and 3.12 too; 3.9-3.11 and 3.14 have no private libpython.)
#  - host/cmake*/bin/cmake-gui: Qt GUI, never used by the builds.
#  - wch/ch32fun/examples_usb/**/testtop*: example test programs, not tools.
OPTIONAL_UNUSED = (
    ("-esp-elf-gdb-3.", ("-3.9", "-3.10", "-3.11", "-3.14")),
    ("/bin/cmake-gui", ("",)),
    ("wch/ch32fun/examples_usb/", ("",)),
)
WORKSPACE = os.path.realpath(os.path.join(os.path.dirname(__file__), ".."))
SKIP_DIRS = {".git", "sysroot", "sysroots", "picolibc", "newlib", "lib/gcc", "share"}


def is_host_elf(path):
    try:
        with open(path, "rb") as f:
            head = f.read(20)
    except OSError:
        return False
    # ELF, 64-bit, little endian, e_machine x86-64 (0x3e)
    return head[:4] == b"\x7fELF" and head[4] == 2 and head[18] == 0x3E


def walk(root):
    for d, dirs, files in os.walk(root):
        dirs[:] = [x for x in dirs if x not in SKIP_DIRS and not x.startswith(".")]
        for name in files:
            p = os.path.join(d, name)
            if os.path.islink(p) or not os.access(p, os.X_OK):
                continue
            if is_host_elf(p):
                yield p


def deps(path):
    try:
        out = subprocess.run(["ldd", path], capture_output=True, text=True, timeout=30).stdout
    except (OSError, subprocess.TimeoutExpired):
        return []
    res = []
    for line in out.splitlines():
        line = line.strip()
        if "=>" in line:
            name, _, rest = line.partition("=>")
            target = rest.strip().split(" (")[0].strip()
            res.append((os.path.basename(name.strip()), target))
        elif line.startswith("/") or line.startswith("linux-vdso"):
            res.append((os.path.basename(line.split(" (")[0]), line.split(" (")[0]))
    return res


def main(roots):
    bad = 0
    checked = 0
    gcc_rt = set()
    optional = []
    for root in roots:
        for exe in walk(root):
            checked += 1
            problems = []
            for name, target in deps(exe):
                if target == "not found":
                    problems.append(f"{name} NOT FOUND")
                elif target.startswith(WORKSPACE) or name in GLIBC:
                    continue
                elif name in GCC_RT:
                    gcc_rt.add(os.path.relpath(exe, WORKSPACE))
                else:
                    problems.append(f"{name} -> {target}")
            rel = os.path.relpath(exe, WORKSPACE)
            if problems and any(k in rel and rel.endswith(tuple(e)) if e[0] else k in rel
                                for k, e in OPTIONAL_UNUSED):
                optional.append(rel)
                continue
            if problems:
                bad += 1
                print(f"OS-LIB {os.path.relpath(exe, WORKSPACE)}: {', '.join(problems)}")
    for o in optional:
        print(f"optional, unused: {o}")
    print(f"checked {checked} host binaries: {bad} need OS libraries, {len(gcc_rt)} use gcc runtime, {len(optional)} optional/unused")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:] or [WORKSPACE]))
