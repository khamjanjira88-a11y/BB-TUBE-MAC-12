#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PLUGIN="$(find "$ROOT/MAC_OS/build" -type d -name "BB Tube Compressor.component" -print -quit)"
if [ -z "$PLUGIN" ]; then
  echo "AU not found. Run MAC_OS/Scripts/build.sh first."
  exit 1
fi
DEST="$HOME/Library/Audio/Plug-Ins/Components/BB Tube Compressor.component"
mkdir -p "$(dirname "$DEST")"
rm -rf "$DEST"
cp -R "$PLUGIN" "$DEST"
echo "Installed: $DEST"
