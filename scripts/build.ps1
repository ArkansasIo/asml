# ============================================================================
#  ASMLEX - configure + build helper
#
#    powershell -ExecutionPolicy Bypass -File scripts\build.ps1
#    powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Clean
#
#  Locates MSVC through vswhere, imports the developer environment and drives
#  CMake + Ninja.  No Visual Studio Developer Prompt required.
# ============================================================================
param(
    [switch]$Clean,
    [switch]$Debug,
    [switch]$Run,
    [string]$Target = "asmlex"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root "build"
$config = if ($Debug) { "Debug" } else { "Release" }

function Find-VsWhere {
    $candidates = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
        "$env:ProgramFiles\Microsoft Visual Studio\Installer\vswhere.exe"
    )
    foreach ($c in $candidates) { if (Test-Path $c) { return $c } }
    throw "vswhere.exe not found - install the Visual Studio Build Tools."
}

$vswhere = Find-VsWhere
$vsPath = & $vswhere -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if (-not $vsPath) { throw "No MSVC toolset found. Install 'Desktop development with C++'." }
$vsPath = $vsPath.Trim()

# --- import the developer environment --------------------------------------
$devShell = Join-Path $vsPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
if (Test-Path $devShell) {
    Import-Module $devShell
    Enter-VsDevShell -VsInstallPath $vsPath -SkipAutomaticLocation `
                     -DevCmdArguments "-arch=x64 -host_arch=x64" | Out-Null
} else {
    throw "Microsoft.VisualStudio.DevShell.dll missing at $devShell"
}

# --- locate the tools we need ----------------------------------------------
$cmake = Join-Path $env:ProgramFiles "CMake\bin\cmake.exe"
if (-not (Test-Path $cmake)) { $cmake = "cmake" }

$ninjaCmd = Get-Command ninja -ErrorAction SilentlyContinue
$ninja = $null
if ($ninjaCmd) {
    $ninja = $ninjaCmd.Source
} else {
    $candidate = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    if (Test-Path $candidate) { $ninja = $candidate }
}
if ($ninja) { $env:Path = (Split-Path -Parent $ninja) + ";" + $env:Path }

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "[asmlex] removing $buildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

# --- configure ---------------------------------------------------------------
$genArgs = @("-S", $root, "-B", $buildDir,
             "-DCMAKE_BUILD_TYPE=$config",
             "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
if ($ninja) { $genArgs += @("-G", "Ninja") } else { $genArgs += @("-G", "Visual Studio 17 2022", "-A", "x64") }

Write-Host "[asmlex] configuring ($config)" -ForegroundColor Cyan
& $cmake @genArgs
if ($LASTEXITCODE -ne 0) { throw "cmake configure failed ($LASTEXITCODE)" }

# --- build -------------------------------------------------------------------
Write-Host "[asmlex] building target '$Target'" -ForegroundColor Cyan
& $cmake --build $buildDir --target $Target --config $config
if ($LASTEXITCODE -ne 0) { throw "cmake build failed ($LASTEXITCODE)" }

$exe = Join-Path $buildDir "bin\asmlex.exe"
if (Test-Path $exe) {
    Write-Host "[asmlex] built -> $exe" -ForegroundColor Green
    if ($Run) { Start-Process -FilePath $exe -WorkingDirectory $root }
} else {
    Write-Host "[asmlex] build finished but $exe was not found" -ForegroundColor Red
}
