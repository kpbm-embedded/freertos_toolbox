#!/usr/bin/env bash
# Usage: host/patch-rpath.sh <file-or-dir> ...   (needs host/env.sh sourced for patchelf)
# For every x86-64 ELF that resolves a library from the OS (or not at all) and that library exists in
# mcu/host/lib (or the host CPython lib dir), add an $ORIGIN-relative RUNPATH to it. Idempotent.
set -u
HOST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Libraries of <elf> that resolve from the OS or are not found.
os_libraries() {
  ldd "$1" 2>/dev/null | grep -E 'not found|=> /usr/lib|=> /lib' | awk '{print $1}'
}

# Directory in host/ that provides library <name> ("" for glibc/gcc runtime or when host/ lacks it).
host_library_dir() {
  local name="$1" version python_lib
  case "$name" in
    libpython3.*)
      version="${name#libpython}"
      version="${version%%.so*}"
      python_lib="$HOST_DIR/python/cpython-$version-linux-x86_64-gnu/lib"
      if [ -e "$python_lib/$name" ]; then echo "$python_lib"; fi
      ;;
    libc.so* | libm.so* | libdl.so* | libpthread.so* | librt.so* | libstdc++* | libgcc_s* | ld-linux* | libutil*) ;;
    *)
      if [ -e "$HOST_DIR/lib/$name" ]; then echo "$HOST_DIR/lib"; fi
      ;;
  esac
}

# Add the needed $ORIGIN-relative RUNPATH entries to one file (ELF files only).
patch_one() {
  local file="$1" origin library dir runpath="" current
  head -c4 "$file" | grep -q ELF || return 0
  origin="$(dirname "$(readlink -f "$file")")"
  for library in $(os_libraries "$file"); do
    dir="$(host_library_dir "$library")"
    [ -n "$dir" ] && runpath="$runpath:\$ORIGIN/$(realpath -m --relative-to="$origin" "$dir")"
  done
  [ -n "$runpath" ] || return 0
  runpath="$(echo "${runpath#:}" | tr ':' '\n' | sort -u | paste -sd:)"
  current="$(patchelf --print-rpath "$file" 2>/dev/null)"
  [ -n "$current" ] && runpath="$current:$runpath"
  patchelf --set-rpath "$runpath" "$file" && echo "patched $file"
}

# Executables and shared objects under <dir> (scripts and static archives skipped).
candidate_files() {
  find "$1" -type f \( -perm -u+x -o -name '*.so*' \) -not -name '*.sh' -not -name '*.py' -not -name '*.a' -not -name '*.la'
}

for arg in "$@"; do
  if [ -d "$arg" ]; then
    candidate_files "$arg" | while read -r file; do patch_one "$file"; done
  else
    patch_one "$arg"
  fi
done
