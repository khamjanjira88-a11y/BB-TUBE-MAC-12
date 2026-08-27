#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cmake -S "$ROOT" -B "$ROOT/MAC_OS/build" -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DBB_BUILD_VST3=ON \
  -DBB_BUILD_AU=ON
echo "Xcode project generated: $ROOT/MAC_OS/build"
