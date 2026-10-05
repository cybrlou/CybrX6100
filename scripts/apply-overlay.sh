#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
# Apply the CybrX6100 overlay + patches onto local upstream checkouts.
#
#   scripts/apply-overlay.sh --br /path/AetherX6100Buildroot [--gui /path/x6100_gui] [--mode copy|patch] [--dry-run]
#
#   Upstream pins: AetherX6100Buildroot @ 07a8d25, x6100_gui @ 02f5240
#   --mode copy  (default) rsync overlay/ into <br>/br2_external/ (full files)
#   --mode patch git apply patches/buildroot/*.patch, then copy only Cybr-new files
#   --gui        git am patches/gui/*.patch onto the GUI checkout and write build/local.mk
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BR=""; GUI=""; MODE=copy; DRY=0
BR_PIN=07a8d25; GUI_PIN=02f5240
while [ $# -gt 0 ]; do
  case "$1" in
    --br) BR="$2"; shift 2 ;;
    --gui) GUI="$2"; shift 2 ;;
    --mode) MODE="$2"; shift 2 ;;
    --dry-run) DRY=1; shift ;;
    -h|--help) sed -n 2,12p "$0"; exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done
[ -n "$BR" ] && [ -d "$BR/br2_external" ] || { echo "need --br <AetherX6100Buildroot checkout>" >&2; exit 2; }
run() { if [ "$DRY" = 1 ]; then echo "+ $*"; else "$@"; fi; }

warn_pin() { # repo pin
  local head; head=$(git -C "$1" rev-parse --short=7 HEAD 2>/dev/null || echo "?")
  [ "$head" = "$2" ] || echo "WARN: $1 is at $head, overlay was made against $2" >&2
}
warn_pin "$BR" "$BR_PIN"

case "$MODE" in
  copy)
    run rsync -a --exclude '*.bak.*' "$ROOT/overlay/" "$BR/br2_external/" ;;
  patch)
    for p in "$ROOT"/patches/buildroot/*.patch; do run git -C "$BR" apply "$p"; done
    # files that do not exist upstream (untracked Cybr additions)
    ( cd "$ROOT/overlay" && find . -type f ) | while read -r f; do
      [ -e "$BR/br2_external/$f" ] && continue
      run install -D -m "$(stat -c %a "$ROOT/overlay/$f")" "$ROOT/overlay/$f" "$BR/br2_external/$f"
    done ;;
  *) echo "bad --mode $MODE" >&2; exit 2 ;;
esac
echo "overlay applied to $BR ($MODE)"

if [ -n "$GUI" ]; then
  warn_pin "$GUI" "$GUI_PIN"
  run git -C "$GUI" am "$ROOT"/patches/gui/*.patch
  run mkdir -p "$BR/build"
  if [ "$DRY" = 1 ]; then echo "+ write $BR/build/local.mk"; else
    echo "X6100_GUI_OVERRIDE_SRCDIR = $(cd "$GUI" && pwd)" > "$BR/build/local.mk"; fi
  echo "GUI patched; local.mk -> $GUI"
fi
