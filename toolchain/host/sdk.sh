# shellcheck shell=bash
# host layer plugin (see ../PLUGIN.md). No SDK_SELFCHECK: the host layer is what selfcheck itself runs on.
SDK_DESC="Shared host layer: Yocto buildtools, cmake, ninja, uv, managed CPython, private libs (zstd zlib ncurses libusb eudev patchelf)"

sdk_env() { source "$SDK_DIR/env.sh"; }

# Every entry is side by side: <name>-<version>/ plus a stable symlink <name>. env.sh only names the symlinks,
# so flipping one never needs an env.sh edit and host/env.sh keeps working while an update runs.
sdk_prune_families() {
  cat <<'F'
buildtools|buildtools-[0-9]*
cmake|cmake-[0-9]*
ninja|ninja-[0-9]*
uv|uv-[0-9]*
patchelf|patchelf-[0-9]*
F
}

_h_have() { command -v "$1" >/dev/null 2>&1; }
sdk_doctor() {
  local rc=0 t
  for t in "git --version" "make --version" "gcc --version" "tar --version" "patch --version" "wget --version" "m4 --version" "pkg-config --version" \
           "cmake --version" "ninja --version" "uv --version" "patchelf --version" "python3 --version"; do
    set -- $t
    if _h_have "$1"; then printf 'ok      %-11s %s\n' "$1" "$($t 2>&1 | head -1)"; else echo "MISSING $1"; rc=1; fi
  done
  local p; for p in 3.8 3.12 3.13; do
    [[ -x $SDK_DIR/python/cpython-$p-linux-x86_64-gnu/bin/python3 ]] && echo "ok      cpython-$p  $("$SDK_DIR/python/cpython-$p-linux-x86_64-gnu/bin/python3" --version)" || { echo "MISSING cpython-$p"; rc=1; }
  done
  local l; for l in libzstd.so.1 libz.so.1 libncursesw.so.6 libusb-1.0.so.0 libudev.so.1; do
    [[ -e $SDK_DIR/lib/$l ]] && echo "ok      lib/$l" || { echo "MISSING lib/$l"; rc=1; }
  done
  return $rc
}

_h_bt_ver() { sed -n 's/^Distro Version: *//p' "$SDK_DIR/buildtools/version-x86_64-pokysdk-linux"; }
_h_dl_ver() { # <glob prefix in dl/> -> version of the source tarball the lib was built from
  ls "$SDK_DIR"/dl/"$1"-*.tar.* 2>/dev/null | sed -E "s#.*/$1-([0-9][0-9.]*[0-9])\.tar.*#\1#" | sort -V | tail -1
}
_h_py_ver() { basename "$(readlink -f "$SDK_DIR/python/cpython-$1-linux-x86_64-gnu")" | sed -E 's/cpython-([0-9.]+)-linux.*/\1/'; }
sdk_versions() {
  echo "buildtools-extended $(_h_bt_ver)  (gcc $(gcc -dumpfullversion 2>/dev/null), git $(git --version | awk '{print $3}'), make $(make --version | head -1 | awk '{print $NF}'))"
  echo "cmake $(cmake --version | head -1 | awk '{print $3}')"
  echo "ninja $(ninja --version)"
  echo "uv $(uv --version | awk '{print $2}')"
  echo "cpython $(_h_py_ver 3.8) $(_h_py_ver 3.12) $(_h_py_ver 3.13)"
  echo "zstd $(_h_dl_ver zstd)  zlib $(_h_dl_ver zlib)  ncurses $(_h_dl_ver ncurses)  libusb $(_h_dl_ver libusb)  eudev $(_h_dl_ver eudev)  patchelf $(patchelf --version | awk '{print $2}')"
}

# ---- installers (each leaves the old version in place and flips the stable link last) ----
_h_install_archive() { # <name> <ver> <url> <sha-spec> <archive-name> <stable> -> extract with strip 1 into host/<name>-<ver>
  local name="$1" ver="$2" url="$3" sha="$4" arc="$5" stable="$6" dest="$SDK_DIR/$1-$2"
  upd_fetch "$url" "$SDK_DIR/dl/$arc" "$sha" || return 1
  upd_unpack "$SDK_DIR/dl/$arc" "$dest" 1 || return 1
  [[ -L $SDK_DIR/$stable ]] || { echo "  $stable is not a symlink; run the one-time adoption described in host/README.md" >&2; return 1; }
  upd_flip "$SDK_DIR/$stable" "$name-$ver"
}
_h_gh_pair() { # <repo> -> "tag version"
  local t; t="$(upd_gh_latest "$1")"; [[ -n $t ]] && echo "$t ${t#v}"
}
_h_up_cmake() {
  local t v url sha; read -r t v <<<"$(_h_gh_pair Kitware/CMake)"
  upd_step cmake "$(cmake --version | head -1 | awk '{print $3}')" "$v" _h_do_cmake "$v"
}
_h_do_cmake() { local v="$1" a="cmake-$1-linux-x86_64.tar.gz" b="https://github.com/Kitware/CMake/releases/download/v$1"
  local h; h="$(upd_curl -m 60 "$b/cmake-$v-SHA-256.txt" | grep " $a\$" | cut -d' ' -f1)"
  _h_install_archive cmake "$v" "$b/$a" "hex:$h" "$a" cmake; }
_h_up_uv() {
  local t v; read -r t v <<<"$(_h_gh_pair astral-sh/uv)"
  upd_step uv "$(uv --version | awk '{print $2}')" "$v" _h_do_uv "$v"
}
_h_do_uv() { local v="$1" a="uv-x86_64-unknown-linux-gnu.tar.gz" b="https://github.com/astral-sh/uv/releases/download/$1"
  _h_install_archive uv "$v" "$b/$a" "url:$b/$a.sha256" "uv-$v-$a" uv; }
_h_up_ninja() {
  local t v; read -r t v <<<"$(_h_gh_pair ninja-build/ninja)"
  upd_step ninja "$(ninja --version)" "$v" _h_do_ninja "$t" "$v"
}
_h_do_ninja() { local t="$1" v="$2" info url dg dest="$SDK_DIR/ninja-$2"
  info="$(upd_gh_asset ninja-build/ninja "$t" 'ninja-linux\.zip')"; IFS=$'\t' read -r url dg _ <<<"$info"
  upd_fetch "$url" "$SDK_DIR/dl/ninja-$v-linux.zip" "${dg:+hex:$dg}" || return 1
  if [[ ! -d $dest ]]; then mkdir -p "$dest.part" && python3 -I -c 'import zipfile,sys;zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])' "$SDK_DIR/dl/ninja-$v-linux.zip" "$dest.part" &&
    chmod +x "$dest.part/ninja" && mv -T "$dest.part" "$dest" || return 1; fi
  upd_flip "$SDK_DIR/ninja" "ninja-$v"; }
_h_up_patchelf() {
  local t v; read -r t v <<<"$(_h_gh_pair NixOS/patchelf)"
  upd_step patchelf "$(patchelf --version | awk '{print $2}')" "$v" _h_do_patchelf "$t" "$v"
}
_h_do_patchelf() { local t="$1" v="$2" info url dg src="$MCU_OUT/src/patchelf-$2"
  info="$(upd_gh_asset NixOS/patchelf "$t" "patchelf-$v\\.tar\\.gz")"; IFS=$'\t' read -r url dg _ <<<"$info"
  upd_fetch "$url" "$SDK_DIR/dl/patchelf-$v.tar.gz" "${dg:+hex:$dg}" || return 1
  mkdir -p "$MCU_OUT/src"; [[ -d $src ]] || upd_unpack "$SDK_DIR/dl/patchelf-$v.tar.gz" "$src" 1 || return 1
  ( cd "$src" && ./configure --prefix="$MCU_OUT/stage/patchelf-$v" >/dev/null && make -j"$MCU_JOBS" >/dev/null && make install >/dev/null ) || return 1
  mkdir -p "$SDK_DIR/patchelf-$v" && cp "$MCU_OUT/stage/patchelf-$v/bin/patchelf" "$SDK_DIR/patchelf-$v/" || return 1
  upd_flip "$SDK_DIR/patchelf" "patchelf-$v"; }

_h_up_buildtools() {
  local cur minor idx latest url
  cur="$(_h_bt_ver)"; minor="${cur%.*}"
  idx="$(upd_curl -m 30 https://downloads.yoctoproject.org/releases/yocto/ 2>/dev/null | grep -oE "yocto-${minor//./\\.}\.[0-9]+/" | tr -d / | sed 's/yocto-//' | sort -V | tail -1)"
  latest="$idx"
  upd_step buildtools-extended "$cur" "$latest" _h_do_buildtools "$latest"
}
_h_do_buildtools() { local v="$1" a="x86_64-buildtools-extended-nativesdk-standalone-$1.sh" b="https://downloads.yoctoproject.org/releases/yocto/yocto-$1/buildtools"
  local dest="$SDK_DIR/buildtools-$v" cur; cur="$(_h_bt_ver)"
  upd_fetch "$b/$a" "$SDK_DIR/dl/$a" "url:$b/$a.sha256sum" || return 1
  if [[ ! -d $dest ]]; then
    sh "$SDK_DIR/dl/$a" -y -d "$dest.part" >/dev/null 2>&1 || { echo "  buildtools installer failed" >&2; return 1; }
    # the installer bakes its target path into the SDK, so install under the final name
    mv -T "$dest.part" "$dest" 2>/dev/null || true
  fi
  if [[ -d $SDK_DIR/buildtools && ! -L $SDK_DIR/buildtools ]]; then
    mv -T "$SDK_DIR/buildtools" "$SDK_DIR/buildtools-$cur" && ln -s "buildtools-$cur" "$SDK_DIR/buildtools"   # adopt (one-time; paths baked in resolve via the link)
  fi
  [[ -x $dest/environment-setup-x86_64-pokysdk-linux || -f $dest/environment-setup-x86_64-pokysdk-linux ]] || return 1
  upd_flip "$SDK_DIR/buildtools" "buildtools-$v"; }

_h_up_python() {
  local m cur new
  local dl; dl="$(uv python list --only-downloads 2>/dev/null)"
  for m in 3.8 3.12 3.13; do
    cur="$(_h_py_ver $m)"
    new="$(grep -oE "cpython-${m//./\\.}\.[0-9]+-linux-x86_64-gnu " <<<"$dl" | sed -E 's/cpython-([0-9.]+)-.*/\1/' | sort -V | tail -1)"
    upd_step "cpython-$m" "$cur" "$new" _h_do_python "$m" "$new"
  done
}
_h_do_python() { local m="$1" v="$2" link="python/cpython-$1-linux-x86_64-gnu" prev
  prev="$(readlink "$SDK_DIR/$link")"
  upd_rec link "$link" "$prev" "$v"
  uv python install --no-bin "$v" || return 1
  # uv keeps the minor-version link current; make sure, since env.sh and the RUNPATHs name the minor link
  [[ "$(readlink "$SDK_DIR/$link")" == *"cpython-$v-"* ]] || ln -sfn "$SDK_DIR/python/cpython-$v-linux-x86_64-gnu" "$SDK_DIR/$link"
  [[ -x $SDK_DIR/$link/bin/python3 ]]; }

_h_up_libs() {
  local v n
  _lib() { upd_report "lib:$1" "$(_h_dl_ver "$1")" "$2"; }
  v="$(_h_gh_pair facebook/zstd | cut -d' ' -f2)"; _lib zstd "$v"
  v="$(upd_git_newest_tag https://github.com/madler/zlib '^v[0-9]+\.[0-9]+(\.[0-9]+)?$')"; _lib zlib "${v#v}"
  v="$(upd_curl -m 30 https://ftp.gnu.org/gnu/ncurses/ 2>/dev/null | grep -oE 'ncurses-[0-9]+\.[0-9]+\.tar\.gz"' | sed -E 's/ncurses-(.*)\.tar.*/\1/' | sort -V | tail -1)"; _lib ncurses "$v"
  v="$(_h_gh_pair libusb/libusb | cut -d' ' -f2)"; _lib libusb "$v"
  v="$(_h_gh_pair eudev-project/eudev | cut -d' ' -f2)"; _lib eudev "$v"
  # A rebuild changes the ABI behind every RUNPATH'd vendor binary, so it is never automatic: the libs are only reported.
  # MCU_YES=1 does not rebuild them (flags are in README.md); rebuild by hand into host/lib, then run host/patch-rpath.sh.
  if upd_yes; then echo "  (private libs are report-only: rebuild by hand, see README.md)"; fi
}

sdk_update() {
  UPD_RC=0
  echo "-- Yocto buildtools-extended";   _h_up_buildtools
  echo "-- cmake / ninja / uv / patchelf"; _h_up_cmake; _h_up_ninja; _h_up_uv; _h_up_patchelf
  echo "-- managed CPython (uv python install --no-bin)"; _h_up_python
  echo "-- private libs in host/lib"; _h_up_libs
  upd_dry_hint
  return "$UPD_RC"
}
