# อัปโหลด BB Tube Compressor ขึ้น GitHub

## 1) สร้าง Repository
GitHub → New repository → ตั้งชื่อเช่น:
`BB-Tube-Compressor`

## 2) อัปโหลดไฟล์
อัปโหลด “ไฟล์ด้านในโฟลเดอร์นี้” ทั้งหมดลง root ของ repository

ต้องเห็น:
```text
.github/
MAC_OS/
WINDOWS/
Source/
Presets/
Docs/
CMakeLists.txt
README.md
```

## 3) Commit
Commit ไปที่ branch `main`

## 4) Build macOS
GitHub → Actions → `BB Tube Compressor - macOS` → Run workflow

ผลลัพธ์:
- VST3
- AU
- macOS `.pkg`

## 5) Build Windows
GitHub → Actions → `BB Tube Compressor - Windows` → Run workflow

ผลลัพธ์:
- Windows x64 VST3

## 6) ดาวน์โหลดผลลัพธ์
เปิด workflow run ที่เสร็จแล้ว → Artifacts → Download
