#!/usr/bin/env bash
# Safely copy the actual NEC VX-2 board port and current coreboot config into this repository.
# Usage: bash scripts/snapshot-local.sh /path/to/coreboot
set -euo pipefail
SRC="${1:-}"
if [[ -z "$SRC" || ! -f "$SRC/.config" ]]; then
  printf 'Usage: %s /path/to/coreboot\n' "$0" >&2
  exit 2
fi
SRC="$(cd "$SRC" && pwd)"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BOARD="$SRC/src/mainboard/nec/versapro_vx2"
if [[ ! -d "$BOARD" ]]; then
  printf 'Cannot find board directory: %s\n' "$BOARD" >&2
  exit 2
fi
mkdir -p "$ROOT/config" "$ROOT/board-port"
cp "$SRC/.config" "$ROOT/config/coreboot.config"
# Source-only copy, avoids accidentally bundling proprietary firmware or build artifacts.
find "$BOARD" -type f \( -name '*.c' -o -name '*.h' -o -name '*.asl' -o -name '*.ads' -o -name '*.adb' -o -name '*.cb' -o -name 'Kconfig*' -o -name 'Makefile*' -o -name '*.md' -o -name '*.txt' -o -name '*.s' -o -name '*.S' -o -name '*.dts' -o -name '*.json' \) -print0 |
  while IFS= read -r -d '' f; do
    rel="${f#"$BOARD"/}"
    mkdir -p "$ROOT/board-port/$(dirname "$rel")"
    cp "$f" "$ROOT/board-port/$rel"
  done
{
  printf '# Build snapshot provenance\n\n'
  printf 'Generated: %s\n\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  printf 'Source path (local only): %s\n\n' "$SRC"
  if git -C "$SRC" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'Base revision: `%s`\n\n' "$(git -C "$SRC" rev-parse HEAD)"
    printf 'Working tree status at snapshot time:\n\n```text\n'
    git -C "$SRC" status --short -- src/mainboard/nec/versapro_vx2 || true
    printf '```\n'
  fi
  printf '\nFiles copied: board source files and complete `.config` only. OEM binaries excluded.\n'
} > "$ROOT/docs/build-provenance.md"
printf 'Snapshot complete. Review config/coreboot.config and board-port/ before publishing.\n'
printf 'IMPORTANT: inspect the config for private identifiers and local paths.\n'
