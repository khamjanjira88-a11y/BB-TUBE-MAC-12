# macOS — BB Tube Compressor

## Requirements
- macOS + Xcode
- CMake 3.22+
- Internet for first configure (JUCE is fetched by CMake)

## Build Xcode project
```bash
chmod +x MAC_OS/Scripts/*.sh MAC_OS/Installer/build_pkg.sh
MAC_OS/Scripts/configure_xcode.sh
open MAC_OS/build/BBTubeCompressor.xcodeproj
```

In Xcode:
1. Scheme: BBTubeCompressor
2. Destination: My Mac
3. Product > Build (⌘B)
4. Configuration: Release for distribution builds

Universal 2 is configured:
`arm64;x86_64`

## Find output
```bash
find MAC_OS/build -type d \( -name "*.vst3" -o -name "*.component" \) -print
```

## Install
```bash
MAC_OS/Scripts/install_vst3.sh
MAC_OS/Scripts/install_au.sh
```

VST3:
`~/Library/Audio/Plug-Ins/VST3/`

AU:
`~/Library/Audio/Plug-Ins/Components/`

## Build installer
```bash
MAC_OS/Installer/build_pkg.sh
```

For commercial shipping, sign the plugin and installer with Apple Developer certificates and complete notarization.
