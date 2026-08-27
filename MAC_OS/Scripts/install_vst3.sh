#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PLUGIN="$(find "$ROOT/MAC_OS/build" -type d -name "BB Tube Compressor.vst3" -print -quit)"
if [ -z "$PLUGIN" ]; then
  echo "VST3 not found. Run MAC_OS/Scripts/build.sh first."
  exit 1
fi
DEST="$HOME/Library/Audio/Plug-Ins/VST3/BB Tube Compressor.vst3"
mkdir -p "$(dirname "$DEST")"
rm -rf "$DEST"
cp -R "$PLUGIN" "$DEST"
echo "Installed: $DEST"
