param([string]$Gpp = "C:\mingw64\bin\g++.exe", [string]$Csc = "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe")
$ErrorActionPreference = "Stop"
$packageRoot = Split-Path $PSScriptRoot -Parent
Push-Location $packageRoot
try {
    & $Gpp -std=c++17 -shared -O2 -static source/bridge.cpp source/accent.cpp source/icons.cpp -o engine/theme_engine.dll
    if ($LASTEXITCODE -ne 0) { throw "Engine build failed" }
    & $Gpp -D_WIN32_WINNT=0x0600 -std=c++17 -municode -O2 -static source/helper.cpp -lversion -o engine/theme-helper.exe
    if ($LASTEXITCODE -ne 0) { throw "Helper build failed" }
    & $Csc /nologo /target:winexe /platform:x64 /optimize+ /win32icon:source\lakeskin.ico /out:LakeSkin.exe /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /reference:System.Web.Extensions.dll source\Studio.cs
    if ($LASTEXITCODE -ne 0) { throw "GUI build failed" }
    Write-Output "LakeSkin build completed."
} finally { Pop-Location }
