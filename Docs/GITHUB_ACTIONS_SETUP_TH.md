# GitHub Actions — BB Tube Compressor

## Repository structure

```text
.github/workflows/build-macos.yml
.github/workflows/build-windows.yml
Source/
MAC_OS/
WINDOWS/
Presets/
CMakeLists.txt
README.md
```

## macOS workflow

1. Push the repository to GitHub.
2. Open Actions.
3. Select `BB Tube Compressor - macOS`.
4. Press `Run workflow` for a manual run.
5. After success, open the run.
6. Download `BB-Tube-Compressor-macOS-<commit>`.

It contains:
- Universal 2 VST3
- AU when `BB_BUILD_AU=ON`
- unsigned development `.pkg`

## Windows workflow

1. Open Actions.
2. Select `BB Tube Compressor - Windows`.
3. Press `Run workflow`.
4. Download `BB-Tube-Compressor-Windows-<commit>`.

It contains:
- Windows x64 VST3 ZIP
- Windows installer EXE only when Inno Setup is available
- build documentation

## Commercial signing

Do not put Apple signing certificates or Windows signing certificates directly into the workflow file.

Use GitHub Actions encrypted secrets and/or certificate files generated at build time.

Typical macOS secrets:
- APPLE_TEAM_ID
- APPLE_DEVELOPER_ID_CERT_P12_BASE64
- APPLE_DEVELOPER_ID_CERT_PASSWORD
- APPLE_INSTALLER_CERT_P12_BASE64
- APPLE_INSTALLER_CERT_PASSWORD
- APPLE_NOTARY_KEY_ID
- APPLE_NOTARY_ISSUER
- APPLE_NOTARY_PRIVATE_KEY

Typical Windows secret:
- WINDOWS_PFX_BASE64
- WINDOWS_PFX_PASSWORD

Keep secrets at repository/environment level with least privilege.
