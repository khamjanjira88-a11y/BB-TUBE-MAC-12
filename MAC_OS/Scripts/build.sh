#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
if [ ! -d "$ROOT/MAC_OS/build" ]; then
  "$ROOT/MAC_OS/Scripts/configure_xcode.sh"
fi
cmake --build "$ROOT/MAC_OS/build" --config Release
echo "Build finished."
find "$ROOT/MAC_OS/build" -type d \( -name "*.vst3" -o -name "*.component" \) -print || true
