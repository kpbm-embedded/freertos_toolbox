SDK_DESC="Debug/flash tools: pyocd, probe-rs, xPack OpenOCD, st-flash (+ udev rules in tools/udev)"
sdk_env() { source "$SDK_DIR/env.sh"; }
sdk_doctor() {
  local rc=0 n
  for n in "arm-none-eabi-gcc --version" "cmake --version" "ninja --version" "pyocd --version" "probe-rs --version" "openocd --version" "st-flash --version"; do
    set -- $n; if command -v "$1" >/dev/null; then echo "ok      $1  $($n 2>&1 | head -1)"; else echo "MISSING $1"; rc=1; fi
  done
  command -v dfu-util >/dev/null && echo "ok      dfu-util" || echo "warn    dfu-util not installed (optional)"
  return $rc
}
sdk_versions() {
  echo "arm-gnu-toolchain $(arm-none-eabi-gcc --version|head -1)"; echo "python $(python --version)"; echo "cmake $(cmake --version|head -1)"; echo "pyocd $(pyocd --version 2>&1|head -1)"; echo "probe-rs $(probe-rs --version)"
  echo "xpack-openocd package $(readlink "$SDK_DIR/xpack-openocd")"; echo "openocd $(openocd --version 2>&1|head -1)"; echo "st-flash $(st-flash --version 2>&1|head -1) (stlink built from source)"
}
sdk_selfcheck() { # run under PATH = sysbin only: every tool must start with nothing but glibc + mcu/ libs
  local rc=0 c
  for c in "arm-none-eabi-gcc --version" "arm-none-eabi-gdb --version" "openocd --version" "probe-rs --version" "pyocd --version" "st-flash --version" "cmake --version"; do
    if $c >/dev/null 2>&1; then echo "ok $c"; else echo "FAIL $c"; rc=1; fi
  done
  printf 'int main(void){return 0;}\n' > "$MCU_OUT/selfcheck.c" 2>/dev/null || { mkdir -p "$MCU_OUT"; printf 'int main(void){return 0;}\n' > "$MCU_OUT/selfcheck.c"; }
  arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -c "$MCU_OUT/selfcheck.c" -o "$MCU_OUT/selfcheck.o" && echo "ok arm-none-eabi-gcc compiles" || rc=1
  return $rc
}
sdk_prune_families() {
  cat <<'F'
toolchains/arm-gnu-toolchain|arm-gnu-toolchain-[0-9]*
xpack-openocd|xpack-openocd-[0-9]*
probe-rs|probe-rs-[0-9]*
F
}
_t_arm_installed() { basename "$(readlink -f "$SDK_DIR/toolchains/arm-gnu-toolchain")" | sed 's/arm-gnu-toolchain-//'; }
_t_arm_latest() { # developer.arm.com has no API: probe the release naming scheme (next rel, next minor, next majors)
  local cur M m n c best="" u; cur="$(_t_arm_installed)"
  M="${cur%%.*}"; m="${cur#*.}"; m="${m%%.*}"; n="${cur##*rel}"
  for c in "$M.$m.rel$((n+1))" "$M.$((m+1)).rel1" "$M.$((m+2)).rel1" "$((M+1)).1.rel1" "$((M+1)).2.rel1" "$((M+1)).3.rel1" "$((M+2)).1.rel1" "$((M+2)).2.rel1"; do
    u="https://developer.arm.com/-/media/Files/downloads/gnu/$c/binrel/arm-gnu-toolchain-$c-x86_64-arm-none-eabi.tar.xz.sha256asc"
    curl -sIL -m 25 -o /dev/null -w '%{http_code}' "$u" | grep -q 200 && best="$c"
  done
  echo "${best:-$cur}"
}
_t_do_arm() { local r="$1" f="arm-gnu-toolchain-$1-x86_64-arm-none-eabi.tar.xz" b
  b="https://developer.arm.com/-/media/Files/downloads/gnu/$r/binrel"
  upd_fetch "$b/$f" "$SDK_DIR/dl/arm/$f" "url:$b/$f.sha256asc" || return 1
  upd_unpack "$SDK_DIR/dl/arm/$f" "$SDK_DIR/toolchains/arm-gnu-toolchain-$r" 1 || return 1
  upd_patch "$SDK_DIR/toolchains/arm-gnu-toolchain-$r"
  upd_flip "$SDK_DIR/toolchains/arm-gnu-toolchain" "arm-gnu-toolchain-$r"; }
_t_do_probers() { local t="$1" v="${1#v}" info url dg sha
  info="$(upd_gh_asset probe-rs/probe-rs "$t" 'probe-rs-tools-x86_64-unknown-linux-gnu\.tar\.xz')"; IFS=$'\t' read -r url dg sha <<<"$info"
  upd_fetch "$url" "$SDK_DIR/dl/probe-rs-$v-x86_64-unknown-linux-gnu.tar.xz" "${sha:+url:$sha}" || return 1
  upd_unpack "$SDK_DIR/dl/probe-rs-$v-x86_64-unknown-linux-gnu.tar.xz" "$SDK_DIR/probe-rs-$v" 1 || return 1
  upd_patch "$SDK_DIR/probe-rs-$v"
  upd_flip "$SDK_DIR/probe-rs" "probe-rs-$v"; }
sdk_update() {
  UPD_RC=0
  local cur new info t ver
  cur="$(pyocd --version 2>&1 | head -1)"; new="$(upd_pypi_latest pyocd)"
  upd_step pyocd "$cur" "$new" _t_do_pyocd "$cur"
  cur="$(probe-rs --version | awk '{print $2}')"; t="$(upd_gh_latest probe-rs/probe-rs)"
  upd_step probe-rs "$cur" "${t#v}" _t_do_probers "$t"
  cur="$(sed -n 's/^xpack-openocd-//p' <<<"$(basename "$(readlink -f "$SDK_DIR/xpack-openocd")")")"
  [[ -n "$(readlink "$SDK_DIR/xpack-openocd")" ]] && cur="$(readlink "$SDK_DIR/xpack-openocd" | sed 's/^xpack-openocd-//')"
  info="$(upd_xpack_info xpack-dev-tools/openocd-xpack)"; IFS=$'\t' read -r t ver _ <<<"$info"
  upd_step xpack-openocd "$cur" "$ver" upd_install_xpack xpack-dev-tools/openocd-xpack "$SDK_DIR" xpack-openocd xpack-openocd
  cur="$(_t_arm_installed)"; new="$(_t_arm_latest)"
  upd_step arm-gnu-toolchain "$cur" "$new" _t_do_arm "$new"
  upd_report stlink "$(st-flash --version 2>&1 | head -1 | sed s/^v//)" "$(upd_git_newest_tag https://github.com/stlink-org/stlink '^v[0-9]+\.[0-9]+\.[0-9]+$' | sed 's/^v//')"
  echo "  (stlink is built from source: rebuild by hand, report only)"
  upd_dry_hint
  return "$UPD_RC"
}
_t_do_pyocd() { upd_rec pip "venv/bin/python" "pyocd==${1}"; uv pip install -q --python "$SDK_DIR/venv/bin/python" -U pyocd; }
