#!/usr/bin/env bash
# Smoke test of scripts/app and scripts/setup: help, where, usage errors (exit 2), pins file, setup dry run, and,
# when the core pins are installed, doctor, a template round trip on qemu and two qemu examples.
set -uo pipefail
here=$(cd -P "$(dirname "$0")/.." && pwd -P)
app="$here/scripts/app" setup="$here/scripts/setup"
pass=0 fail=0
t() { local name=$1; shift; if "$@" >/dev/null 2>&1; then pass=$((pass + 1)); else fail=$((fail + 1)); echo "FAIL $name"; fi; }
t help "$app" help
t where "$app" where
t setup-help "$setup" help
t setup-list "$setup" list
t unknown-exits-2 bash -c '"$1" no-such-command; [ $? -eq 2 ]' _ "$app"
t new-no-args-exits-2 bash -c '"$1" new; [ $? -eq 2 ]' _ "$app"
t new-bad-name-exits-2 bash -c '"$1" new qemu-app "bad name" --dir /nonexistent; [ $? -eq 2 ]' _ "$app"
t setup-unknown-exits-2 bash -c '"$1" frobnicate; [ $? -eq 2 ]' _ "$setup"
t setup-nothing-selected-exits-2 bash -c '"$1" check no-such-component; [ $? -eq 2 ]' _ "$setup"
t examples-bad-sub-exits-2 bash -c '"$1" examples frob; [ $? -eq 2 ]' _ "$app"
t pins-9-columns bash -c "awk -F'\t' '!/^#/ && NF && (NF < 8 || NF > 9) {bad=1} END {exit bad}' '$here/config/pins.tsv'"
t pins-sha256-or-git bash -c "awk -F'\t' '!/^#/ && NF && \$5 !~ /^(sha256:[0-9a-f]{64}|git:[0-9a-f]{40})\$/ {bad=1} END {exit bad}' '$here/config/pins.tsv'"
t setup-dry-run bash -c 'SETUP_CACHE=$(mktemp -d) "$1" --dry-run install all'  _ "$setup"
t examples-list "$app" examples list
t templates-list "$app" templates list
t every-template-has-toml bash -c 'for d in "$1"/templates/*/; do [ -f "$d/template.toml" ] || exit 1; done' _ "$here"
t every-example-has-toml bash -c 'for d in "$1"/examples/freertos/*/; do [ -f "$d/example.toml" ] || exit 1; done' _ "$here"
if "$setup" check core >/dev/null 2>&1; then
  tmp=$(mktemp -d "${TMPDIR:-/tmp}/frt-smoke.XXXXXX")
  t doctor "$app" doctor
  t new-qemu-app "$app" new qemu-app smoke_app --dir "$tmp"
  t new-refuses-overwrite bash -c '! "$1" new qemu-app smoke_app --dir "$2"' _ "$app" "$tmp"
  t placeholder-filled bash -c '! grep -rq "@NAME@" "$1/smoke_app" && grep -q "project(smoke_app" "$1/smoke_app/CMakeLists.txt"' _ "$tmp"
  t run-template-on-qemu "$app" run "$tmp/smoke_app"
  t examples-test-two "$app" examples test qemu-queue-pipeline qemu-static-alloc
  # a failing firmware must fail the run (exit 1), not pass silently
  sed -i 's/sum == 55/sum == 56/' "$tmp/smoke_app/src/main.c"
  t failing-firmware-fails bash -c '! "$1" run "$2/smoke_app"' _ "$app" "$tmp"
  echo "smoke: temp files in $tmp"
fi
echo "smoke: $pass ok, $fail failed"
[ "$fail" -eq 0 ]
