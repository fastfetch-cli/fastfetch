#!/usr/bin/env bash
# WARNING: DO NOT RUN THIS SCRIPT UNLESS YOU KNOW WHAT YOU ARE DOING.

set -euo pipefail

command -v debuild >/dev/null 2>&1 || { echo "debuild not found. Install devscripts." >&2; exit 1; }
command -v dput >/dev/null 2>&1 || { echo "dput not found." >&2; exit 1; }

SCRIPT_DIR="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." >/dev/null 2>&1 && pwd)"
OUT_DIR="$(dirname "$ROOT_DIR")"
PPA="ppa:zhangsongcui3371/fastfetch"
CODENAMES=( jammy noble plucky questing resolute )
DRY_RUN=0

if [[ -d "$OUT_DIR/build" ]]; then
    echo "Output directory exists: $OUT_DIR/build" >&2
    echo "Please move or delete it before proceeding." >&2
    exit 1
fi

TEMPLATE="$ROOT_DIR/debian/changelog.tpl"
CHANGELOG_DEBIAN="$ROOT_DIR/debian/changelog"

if [[ ! -f "$TEMPLATE" ]]; then
  echo "Template not found: $TEMPLATE" >&2
  exit 1
fi

if ! grep -q '#UBUNTU_CODENAME#' "$TEMPLATE"; then
  echo "Template missing placeholder: #UBUNTU_CODENAME#" >&2
  exit 1
fi

echo "IMPORTANT: Before proceeding, please ensure that 'debian/changelog.tpl' is up-to-date"
echo "   with the correct version number, release notes, and maintainer information."
echo
echo "   The template must contain a placeholder like '#UBUNTU_CODENAME#' for the codename."
echo
read -p "Have you reviewed and updated 'debian/changelog.tpl'? (y/N): " -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

cleanup() {
    rm -f "$CHANGELOG_DEBIAN"
    rm -f "$ROOT_DIR/debian/files"
}
trap cleanup EXIT

shopt -s nullglob

for codename in "${CODENAMES[@]}"; do
  echo "==> Publishing distro: $codename"
  sed "s/#UBUNTU_CODENAME#/${codename}/g" "$TEMPLATE" > "$CHANGELOG_DEBIAN"

  ( cd "$ROOT_DIR" && debuild -S -i -I )

  changes=( "$OUT_DIR"/fastfetch_*~${codename}_source.changes )
  if [[ ${#changes[@]} -ne 1 ]]; then
    echo "Unable to uniquely identify .changes for '$codename' in: $OUT_DIR" >&2
    printf 'Found:\n'; printf '  %s\n' "${changes[@]}" >&2 || true
    exit 1
  fi

  if [[ $DRY_RUN -eq 1 ]]; then
    echo "[DRY-RUN] dput \"$PPA\" \"${changes[0]}\""
  else
    dput "$PPA" "${changes[0]}"
  fi

  rm -f "$OUT_DIR"/fastfetch_*~${codename}_source.{changes,dsc,tar.*}

  echo "<== Done: $codename"
done
