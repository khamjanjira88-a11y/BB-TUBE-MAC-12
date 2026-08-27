# START HERE

## macOS
```bash
cd /path/to/BB_Tube_Compressor_GitHub_FINAL
chmod +x MAC_OS/Scripts/*.sh MAC_OS/Installer/build_pkg.sh
MAC_OS/Scripts/configure_xcode.sh
open MAC_OS/build/BBTubeCompressor.xcodeproj
```

Then in Xcode:
- Scheme: BBTubeCompressor
- My Mac
- Product > Build

## Windows
```powershell
cd C:\path\to\BB_Tube_Compressor_GitHub_FINAL
Set-ExecutionPolicy -Scope Process Bypass
.\WINDOWS\Scripts\build_x64.ps1
```

## GitHub
Push this repository to GitHub and use the two Actions workflows to produce platform artifacts.
