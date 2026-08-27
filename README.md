# BB Tube Compressor — GitHub Ready

Original BB Audio Tools branded JUCE project for a tube/optical compressor + bus compressor + limiter.

## Formats
- VST3: macOS + Windows
- AU: optional macOS build for Logic Pro
- AAX: not included; requires Avid AAX SDK and separate signing/distribution

## Repository layout
```text
Source/                 shared DSP + GUI
MAC_OS/                 macOS build, install, package
WINDOWS/                Windows build, install, Inno Setup
Presets/                factory preset data
.github/workflows/      separate GitHub Actions
Docs/                   setup guides
```

## Local macOS
```bash
chmod +x MAC_OS/Scripts/*.sh MAC_OS/Installer/build_pkg.sh
MAC_OS/Scripts/configure_xcode.sh
MAC_OS/Scripts/build.sh
MAC_OS/Scripts/install_vst3.sh
MAC_OS/Scripts/install_au.sh
MAC_OS/Installer/build_pkg.sh
```

## Local Windows
```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\WINDOWS\Scripts\build_x64.ps1
.\WINDOWS\Scripts\install_vst3.ps1
```

## GitHub
Upload the contents of this folder to a new repository. Keep `.github` at repository root.
Use Actions:
- BB Tube Compressor — macOS
- BB Tube Compressor — Windows

## Before commercial distribution
Replace placeholder website/email and unique manufacturer/plugin codes. Sign/notarize macOS builds and sign the Windows installer. Test automation, preset recall, sample rates, offline rendering, silence, mono/stereo and target DAWs.

The DSP is an original functional implementation; it is not an exact electrical clone of named third-party hardware.


## Important implementation note
The current DSP provides real peak/RMS/gain-reduction/correlation/balance measurements and a live spectrum/waveform display path when those sources are enabled in the current source tree. It is a functional original dynamics processor, not an exact electrical clone of any third-party hardware.
