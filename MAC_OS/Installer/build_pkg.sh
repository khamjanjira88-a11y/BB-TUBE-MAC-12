#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="$ROOT/MAC_OS/build"
PKGROOT="$ROOT/MAC_OS/Installer/pkgroot"
OUT="$ROOT/MAC_OS/Installer/BB_Tube_Compressor_macOS.pkg"

rm -rf "$PKGROOT" "$OUT"
mkdir -p "$PKGROOT/Library/Audio/Plug-Ins/VST3"

VST3="$(find "$BUILD" -type d -name "BB Tube Compressor.vst3" -print -quit)"
if [ -z "$VST3" ]; then
  echo "VST3 not found. Run MAC_OS/Scripts/build.sh first."
  exit 1
fi
cp -R "$VST3" "$PKGROOT/Library/Audio/Plug-Ins/VST3/"

AU="$(find "$BUILD" -type d -name "BB Tube Compressor.component" -print -quit || true)"
if [ -n "$AU" ]; then
  mkdir -p "$PKGROOT/Library/Audio/Plug-Ins/Components"
  cp -R "$AU" "$PKGROOT/Library/Audio/Plug-Ins/Components/"
fi

pkgbuild \
  --root "$PKGROOT" \
  --identifier "com.bbaudiotools.bbtubecompressor" \
  --version "1.0.0" \
  --install-location "/" \
  "$OUT"

echo "Created: $OUT"
