<#
Builds the firmware of any clone or worktree through one shared "bench" folder.

Why a bench: arduino-builder records absolute paths in its build cache. A fixed project path
and a fixed output folder keep the cache valid, so switching the source folder recompiles only
the sketch files that differ; the core and Libs stay cached. Only "sketches" comes from the
chosen source folder, everything else comes from the main clone.

Windows PowerShell 5.1 compatible. Never deletes anything recursively: the junctions point at
real source folders.

Run it as "powershell -NoProfile -ExecutionPolicy Bypass -File haquda-bench.ps1 ...": with -File
a thrown error ends the process with exit code 1 (with -Command it would not).
#>
[CmdletBinding()]
param(
    # Root of the clone or worktree whose sketches folder is built.
    [Parameter(Mandatory = $true)]
    [string]$Source,

    # Default: <main clone root>\.bench
    [string]$BenchRoot,

    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    # Flash, Monitor and Port join this list in the next stages; last-source.txt is read there too.
    [ValidateSet('Build')]
    [string]$Action = 'Build'
)

$ErrorActionPreference = 'Stop'

$fwRel = 'Software\Software_ver_2\HAQuDA_Esp32'
$projName = 'HAQuDA_Esp32.vgdbproj'
$platform = 'ESP32_Dev_Module'

function Get-MainCloneRoot {
    # The folder that owns the common .git directory, also when this script runs from a worktree.
    # Windows PowerShell 5.1 turns native stderr into errors under 'Stop', so relax it for the call.
    $prev = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $gitDir = & git -C $PSScriptRoot rev-parse --git-common-dir 2>$null
        $gitExit = $LASTEXITCODE
    }
    catch {
        $gitExit = 1
    }
    finally {
        $ErrorActionPreference = $prev
    }
    if ($gitExit -ne 0 -or -not $gitDir) {
        throw "Cannot find the git directory of the repository that holds $PSScriptRoot (is git on PATH?)"
    }
    # A relative answer (".git" in the main clone) is relative to the directory given to -C.
    $gitDir = ([string]$gitDir) -replace '/', '\'
    if (-not [System.IO.Path]::IsPathRooted($gitDir)) { $gitDir = Join-Path $PSScriptRoot $gitDir }
    $gitDir = [System.IO.Path]::GetFullPath($gitDir).TrimEnd('\')
    if ((Split-Path -Leaf $gitDir) -ne '.git') {
        throw "Unsupported repository layout: the common git directory is $gitDir, expected a folder named .git"
    }
    return (Split-Path -Parent $gitDir)
}

# Test-Path answers false for a junction whose target is gone, so read the attributes instead.
function Test-PathEntryExists([string]$path) {
    try {
        [void][System.IO.File]::GetAttributes($path)
        return $true
    }
    catch {
        return $false
    }
}

function Test-IsLink([string]$path) {
    try {
        $attr = [System.IO.File]::GetAttributes($path)
    }
    catch {
        return $false
    }
    return [bool]($attr -band [System.IO.FileAttributes]::ReparsePoint)
}

function Set-Junction([string]$link, [string]$target) {
    $target = [System.IO.Path]::GetFullPath($target).TrimEnd('\')
    if (-not (Test-Path -LiteralPath $target -PathType Container)) {
        throw "Junction target does not exist: $target"
    }
    if (Test-IsLink $link) {
        $item = Get-Item -LiteralPath $link -Force -ErrorAction SilentlyContinue
        $current = if ($item) { $item.Target } else { $null }
        if ($current -is [array]) { $current = $current[0] }
        if ($current -and ($current.TrimEnd('\') -ieq $target)) { return }
        # Non-recursive delete removes only the link (also a dangling one), never what it points at.
        [System.IO.Directory]::Delete($link)
    }
    elseif (Test-PathEntryExists $link) {
        throw "$link exists and is not a junction; remove it by hand"
    }
    New-Item -ItemType Junction -Path $link -Value $target | Out-Null
}

function Update-BenchProject([string]$mainFw, [string]$bench) {
    $src = Join-Path $mainFw $projName
    $dst = Join-Path $bench $projName
    # Latin-1 maps bytes to chars one to one, so everything but the replaced text stays identical.
    $latin1 = [System.Text.Encoding]::GetEncoding(28591)
    $text = $latin1.GetString([System.IO.File]::ReadAllBytes($src))
    $from = '<OutputSubdirectory>Output\$(PlatformName.defuse)'
    $to = '<OutputSubdirectory>build\$(PlatformName.defuse)'
    if (-not $text.Contains($from)) {
        throw "OutputSubdirectory not found in $src; the script must be updated"
    }
    $newBytes = $latin1.GetBytes($text.Replace($from, $to))
    if ((Test-Path -LiteralPath $dst) -and
        ([System.Linq.Enumerable]::SequenceEqual([byte[]][System.IO.File]::ReadAllBytes($dst), [byte[]]$newBytes))) {
        return
    }
    # Written only on change, so VisualGDB is not disturbed by a touched project file.
    [System.IO.File]::WriteAllBytes($dst, $newBytes)
}

function Update-BenchUserFile([string]$mainFw, [string]$bench) {
    # Holds the secret build flags: linked, never read, copied or printed.
    $name = "$projName.$($env:USERNAME)." + 'user'
    $src = Join-Path $mainFw $name
    $dst = Join-Path $bench $name
    if (-not (Test-Path -LiteralPath $src)) {
        throw "The per-user VisualGDB settings file of the main clone is missing ($name in $mainFw). Open the project once in Visual Studio and set the build flags, then run again."
    }
    if ([System.IO.Path]::GetFullPath($src) -ieq [System.IO.Path]::GetFullPath($dst)) {
        throw 'The bench folder must not be the firmware folder of the main clone'
    }
    # Recreated on every run: cheap, and it also repairs a link that an editor broke by
    # saving the original through write+rename. Deleting the bench name never touches the original.
    if (Test-PathEntryExists $dst) { [System.IO.File]::Delete($dst) }
    try {
        New-Item -ItemType HardLink -Path $dst -Value $src -ErrorAction Stop | Out-Null
    }
    catch {
        throw "Cannot hard-link the per-user settings file into the bench. A hard link needs the bench and the main clone on the same volume; choose -BenchRoot on the same drive as $mainFw. ($($_.Exception.Message))"
    }
}

function Find-VisualGdb {
    $candidates = @()
    if (${env:ProgramFiles(x86)}) { $candidates += Join-Path ${env:ProgramFiles(x86)} 'Sysprogs\VisualGDB\VisualGDB.exe' }
    if ($env:ProgramFiles) { $candidates += Join-Path $env:ProgramFiles 'Sysprogs\VisualGDB\VisualGDB.exe' }
    foreach ($c in $candidates) { if (Test-Path -LiteralPath $c) { return $c } }
    $reg = Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Sysprogs\VisualGDB', 'HKLM:\SOFTWARE\Sysprogs\VisualGDB' -ErrorAction SilentlyContinue |
        Where-Object { $_.InstallationPath -or $_.InstallDir } | Select-Object -First 1
    if ($reg) {
        $dir = if ($reg.InstallationPath) { $reg.InstallationPath } else { $reg.InstallDir }
        $exe = Join-Path $dir 'VisualGDB.exe'
        if (Test-Path -LiteralPath $exe) { return $exe }
    }
    throw 'VisualGDB.exe not found. Install VisualGDB or add its folder to the script.'
}

function Initialize-Bench([string]$sourceRoot, [string]$mainRoot, [string]$bench) {
    $mainFw = Join-Path $mainRoot $fwRel
    $srcSketches = Join-Path (Join-Path $sourceRoot $fwRel) 'sketches'
    if (-not (Test-Path -LiteralPath $srcSketches -PathType Container)) {
        throw "No sketches folder at $srcSketches"
    }
    New-Item -ItemType Directory -Force -Path $bench, (Join-Path $bench 'build') | Out-Null
    Update-BenchProject $mainFw $bench
    Update-BenchUserFile $mainFw $bench
    Set-Junction (Join-Path $bench 'Libs') (Join-Path $mainFw 'Libs')
    Set-Junction (Join-Path $bench 'sketches') $srcSketches
    Set-Content -LiteralPath (Join-Path $bench 'last-source.txt') -Value $sourceRoot -Encoding ASCII
}

function Invoke-Build([string]$bench, [string]$cfg) {
    $vg = Find-VisualGdb
    $log = Join-Path $bench "build\$cfg-build.log"
    $errLog = Join-Path $bench "build\$cfg-build.err.log"
    $proj = Join-Path $bench $projName
    $argLine = '/build "{0}" /config:{1} /platform:{2}' -f $proj, $cfg, $platform
    $watch = [System.Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process -FilePath $vg -ArgumentList $argLine -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $log -RedirectStandardError $errLog
    $watch.Stop()
    # The head of the log holds secret -D values, so only selected lines are shown; a line that
    # looks like a command line (compiler flags, prefs) or is very long is never echoed.
    Select-String -Path $log, $errLog -CaseSensitive -SimpleMatch -Pattern 'error:', 'Sketch uses', 'Global variables' |
        ForEach-Object {
            $line = $_.Line
            if ($line.Contains(' -D') -or $line.Contains('-prefs=') -or ($line.Length -gt 300)) {
                Write-Host '<long line suppressed, see log>'
            }
            else {
                Write-Host $line
            }
        }
    Write-Host ('Build exit code {0}, {1:N1} s (log: {2})' -f $p.ExitCode, $watch.Elapsed.TotalSeconds, $log)
    return $p.ExitCode
}

$mainRoot = Get-MainCloneRoot
if (-not $BenchRoot) { $BenchRoot = Join-Path $mainRoot '.bench' }
# Full path before any use: .NET and child processes resolve relative paths against another directory.
if (-not [System.IO.Path]::IsPathRooted($BenchRoot)) { $BenchRoot = Join-Path (Get-Location).ProviderPath $BenchRoot }
$BenchRoot = [System.IO.Path]::GetFullPath($BenchRoot).TrimEnd('\')
$sourceRoot = (Resolve-Path -LiteralPath $Source).Path.TrimEnd('\')

Initialize-Bench $sourceRoot $mainRoot $BenchRoot

switch ($Action) {
    'Build' { exit (Invoke-Build $BenchRoot $Config) }
}
