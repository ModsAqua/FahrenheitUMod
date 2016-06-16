@echo off

if not exist "textures" (
  echo "textures" directory not found!
  pause
  exit 1
)

for /f "tokens=1-6 delims=," %%a in (texalias.txt) do (
  if not exist "textures\%%a.dds" (
    copy /y "textures\%%b.dds" "textures\%%a.dds" 1>nul
    if not "%%c" == "" (
      copy /y "textures\%%b.dds" "textures\%%c.dds" 1>nul
    )
  ) else (
    if not "%%b" == "" (
      copy /y "textures\%%a.dds" "textures\%%b.dds" 1>nul
    )
    if not "%%c" == "" (
      copy /y "textures\%%a.dds" "textures\%%c.dds" 1>nul
    )
    if not "%%d" == "" (
      copy /y "textures\%%a.dds" "textures\%%d.dds" 1>nul
    )
    if not "%%e" == "" (
      copy /y "textures\%%a.dds" "textures\%%e.dds" 1>nul
    )
    if not "%%f" == "" (
      copy /y "textures\%%a.dds" "textures\%%f.dds" 1>nul
    )
  )
)

echo "done"
pause
