# embedded/m3-arch/bringup/build.ps1
# Windows-аналог Makefile: активує ESP-IDF (якщо ще не активований у цій сесії)
# і одразу викликає idf.py — щоб не писати export + idf.py щоразу.
#   .\build.ps1                         # зібрати (build/bringup.elf|.bin)
#   .\build.ps1 flash   -Port COM3      # прошити (Port за замовчуванням нижче)
#   .\build.ps1 monitor -Port COM3
#   .\build.ps1 clean | fullclean | menuconfig | size
param(
    [string]$Target = "build",
    [string]$Port   = "COM3",
    [string]$Idf    = "$env:USERPROFILE\esp\esp-idf"
)
$ErrorActionPreference = "Stop"

# Активувати IDF лише якщо idf.py ще не в PATH (повторна активація не потрібна).
if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    . "$Idf\export.ps1"
}

if ($Target -in @("flash", "monitor")) {
    idf.py -p $Port $Target
} else {
    idf.py $Target
}
