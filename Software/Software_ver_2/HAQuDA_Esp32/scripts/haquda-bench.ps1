<#
Builds, flashes and monitors the firmware of any clone or worktree through one shared "bench" folder.

Why a bench: arduino-builder records absolute paths in its build cache. A fixed project path
and a fixed output folder keep the cache valid, so switching the source folder recompiles only
the sketch files that differ; the core and Libs stay cached. Only "sketches" comes from the
chosen source folder, everything else comes from the main clone.

Actions:
  Build              build the chosen source folder
  Flash              write the last bench build to the lamp over UART (esptool)
  Monitor            serial monitor (pyserial miniterm), exit with Ctrl+]
  BuildFlashMonitor  the three above in a row
  FlashMonitor       Flash, then Monitor
  Port               choose the COM port again

Without -Source a numbered menu lists the main clone and every worktree. The COM port lives in
<bench>\port and is patched into the bench copy of the project only; no tracked file is changed.

Windows PowerShell 5.1 compatible. Never deletes anything recursively: the junctions point at
real source folders.

Run it as "powershell -NoProfile -ExecutionPolicy Bypass -File haquda-bench.ps1 ...": with -File
a thrown error ends the process with exit code 1 (with -Command it would not).
#>
[CmdletBinding()]
param(
    # Root of the clone or worktree whose sketches folder is built. Optional: a menu asks.
    # Flash, Monitor and Port ignore it: they work on the last bench build.
    [string]$Source,

    # Default: <main clone root>\.bench
    [string]$BenchRoot,

    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [ValidateSet('Build', 'Flash', 'Monitor', 'BuildFlashMonitor', 'FlashMonitor', 'Port')]
    [string]$Action = 'Build',

    # COM port to use and remember. Without it the stored port is used.
    [ValidatePattern('^COM\d+$')]
    [string]$Port,

    # Prints the esptool and monitor command lines instead of running them.
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

$fwRel = 'Software\Software_ver_2\HAQuDA_Esp32'
$projName = 'HAQuDA_Esp32.vgdbproj'
$platform = 'ESP32_Dev_Module'
$latin1 = [System.Text.Encoding]::GetEncoding(28591)

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

function Test-SamePath([string]$a, [string]$b) {
    if (-not $a -or -not $b) { return $false }
    return ([System.IO.Path]::GetFullPath($a).TrimEnd('\') -ieq [System.IO.Path]::GetFullPath($b).TrimEnd('\'))
}

function Read-FirstLine([string]$path) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { return $null }
    $line = Get-Content -LiteralPath $path -TotalCount 1 -Encoding UTF8 -ErrorAction SilentlyContinue
    if ($null -eq $line) { return $null }
    $line = ([string]$line).Trim()
    if ($line -eq '') { return $null }
    return $line
}

# Reads a line from the terminal; a closed input counts as an empty answer.
function Read-HostSafe([string]$prompt) {
    try {
        $v = Read-Host $prompt
    }
    catch {
        $v = $null
    }
    if ($null -eq $v) { return '' }
    return ([string]$v).Trim()
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

# ---- Source folder menu -------------------------------------------------------------------

function Get-BranchLabel([string]$gitDir) {
    $headFile = Join-Path $gitDir 'HEAD'
    $line = Read-FirstLine $headFile
    if (-not $line) { return '?' }
    if ($line -match '^ref:\s*refs/heads/(.+)$') { return $Matches[1].Trim() }
    if ($line -match '^[0-9a-f]{7,}') { return ('detached ' + $line.Substring(0, 7)) }
    return '?'
}

# The main clone plus every worktree folder that holds the firmware sketches. Reads the worktree
# registry in .git directly instead of running git.
function Get-SourceCandidates([string]$mainRoot) {
    $list = New-Object System.Collections.ArrayList
    [void]$list.Add([pscustomobject]@{ Root = $mainRoot; Branch = (Get-BranchLabel (Join-Path $mainRoot '.git')) })
    $registry = Join-Path $mainRoot '.git\worktrees'
    if (Test-Path -LiteralPath $registry -PathType Container) {
        foreach ($d in (Get-ChildItem -LiteralPath $registry -Directory)) {
            $gitdirLine = Read-FirstLine (Join-Path $d.FullName 'gitdir')
            if (-not $gitdirLine) { continue }
            # The file holds "<worktree root>/.git".
            $root = Split-Path -Parent (($gitdirLine -replace '/', '\').TrimEnd('\'))
            if (-not $root) { continue }
            if (-not (Test-Path -LiteralPath (Join-Path (Join-Path $root $fwRel) 'sketches') -PathType Container)) { continue }
            [void]$list.Add([pscustomobject]@{ Root = $root; Branch = (Get-BranchLabel $d.FullName) })
        }
    }
    return $list.ToArray()
}

function Select-Source([string]$mainRoot, [string]$bench) {
    $cands = @(Get-SourceCandidates $mainRoot)
    $last = Read-FirstLine (Join-Path $bench 'last-source.txt')
    $default = 1
    for ($i = 0; $i -lt $cands.Count; $i++) {
        if (Test-SamePath $cands[$i].Root $last) { $default = $i + 1 }
    }
    Write-Host 'Folders with firmware sources (* = last used):'
    for ($i = 0; $i -lt $cands.Count; $i++) {
        $mark = if (($i + 1) -eq $default) { '*' } else { ' ' }
        Write-Host ('{0} [{1}] {2,-26} {3}' -f $mark, ($i + 1), $cands[$i].Branch, $cands[$i].Root)
    }
    $answer = Read-HostSafe "Number (Enter = $default)"
    if ($answer -eq '') { return $cands[$default - 1].Root }
    $n = 0
    if (-not [int]::TryParse($answer, [ref]$n) -or $n -lt 1 -or $n -gt $cands.Count) {
        throw "Not a valid choice: $answer"
    }
    return $cands[$n - 1].Root
}

# ---- COM port -----------------------------------------------------------------------------

function Get-ProjectPort([string]$projPath) {
    if (-not (Test-Path -LiteralPath $projPath -PathType Leaf)) { return $null }
    $m = [regex]::Match([System.IO.File]::ReadAllText($projPath, $latin1), '<COMPort>(COM\d+)</COMPort>')
    if ($m.Success) { return $m.Groups[1].Value }
    return $null
}

# Ports that exist now, with the friendly name Windows shows ("USB Serial Port (COM8)").
function Get-PresentPorts {
    $names = @([System.IO.Ports.SerialPort]::GetPortNames() |
        ForEach-Object { ([string]$_).Trim().ToUpperInvariant() } |
        Where-Object { $_ -match '^COM\d+$' } |
        Sort-Object { [int]$_.Substring(3) } -Unique)
    $friendly = @{}
    try {
        $entities = Get-CimInstance -ClassName Win32_PnPEntity -Filter "Name LIKE '%(COM%'" -ErrorAction Stop
        foreach ($e in $entities) {
            if ($e.Name -match '\((COM\d+)\)') { $friendly[$Matches[1].ToUpperInvariant()] = $e.Name }
        }
    }
    catch {
        # Names are a convenience only.
    }
    $result = @()
    foreach ($n in $names) {
        $label = if ($friendly.ContainsKey($n)) { $friendly[$n] } else { $n }
        $result += [pscustomobject]@{ Name = $n; Label = $label }
    }
    return $result
}

function Select-PortFromMenu([string]$current) {
    $ports = @(Get-PresentPorts)
    if ($ports.Count -eq 0) { throw 'No COM ports found. Connect the lamp over USB and run again.' }
    $default = 0
    for ($i = 0; $i -lt $ports.Count; $i++) {
        if ($current -and ($ports[$i].Name -ieq $current)) { $default = $i + 1 }
    }
    Write-Host 'COM ports present:'
    for ($i = 0; $i -lt $ports.Count; $i++) {
        $mark = if (($i + 1) -eq $default) { '*' } else { ' ' }
        Write-Host ('{0} [{1}] {2}' -f $mark, ($i + 1), $ports[$i].Label)
    }
    $prompt = if ($default) { "Number (Enter = $default)" } else { 'Number' }
    $answer = Read-HostSafe $prompt
    if ($answer -eq '' -and $default) { return $ports[$default - 1].Name }
    $n = 0
    if ([int]::TryParse($answer, [ref]$n) -and $n -ge 1 -and $n -le $ports.Count) { return $ports[$n - 1].Name }
    if ($answer -match '^(?i)COM\d+$') { return $answer.ToUpperInvariant() }
    throw "Not a valid choice: $answer"
}

function Save-Port([string]$bench, [string]$port, [string]$mainPort) {
    Set-Content -LiteralPath (Join-Path $bench 'port') -Value $port -Encoding ASCII
    # Remembers which port the VisualGDB project named when the choice was made, to notice a later change.
    if ($mainPort) { Set-Content -LiteralPath (Join-Path $bench 'port-origin') -Value $mainPort -Encoding ASCII }
}

# Returns the port to use (may be $null for a Build with nothing stored and no port in the main project).
# $interactive: ask questions; Build never asks, it only patches what is stored.
function Resolve-Port([string]$mainFw, [string]$bench, [string]$explicit, [bool]$interactive, [bool]$forceMenu) {
    $mainPort = Get-ProjectPort (Join-Path $mainFw $projName)
    $stored = Read-FirstLine (Join-Path $bench 'port')
    $origin = Read-FirstLine (Join-Path $bench 'port-origin')

    if ($explicit) {
        $explicit = $explicit.ToUpperInvariant()
        Save-Port $bench $explicit $mainPort
        if (@(Get-PresentPorts | Where-Object { $_.Name -ieq $explicit }).Count -eq 0) {
            Write-Warning "$explicit is not present now."
        }
        return $explicit
    }

    if (-not $stored) {
        $stored = $mainPort
        if ($stored) { Save-Port $bench $stored $mainPort }
    }
    elseif ($mainPort -and ($mainPort -ine $origin)) {
        # The VisualGDB project names another port than when the bench port was set.
        if ($mainPort -ine $stored -and $interactive) {
            $adopt = Read-HostSafe "VisualGDB project now uses $mainPort, switch to it? [y/N]"
            if ($adopt -match '^(?i)y(es)?$') { $stored = $mainPort }
            Save-Port $bench $stored $mainPort
        }
        elseif ($mainPort -ieq $stored) {
            Save-Port $bench $stored $mainPort
        }
    }

    if (-not $interactive) { return $stored }

    $present = @(Get-PresentPorts | Where-Object { $stored -and ($_.Name -ieq $stored) }).Count -gt 0
    if ($forceMenu -or -not $present) {
        if ($stored -and -not $present) { Write-Host "Stored port $stored is not present." }
        $stored = Select-PortFromMenu $stored
        Save-Port $bench $stored $mainPort
    }
    return $stored
}

# ---- Bench project ------------------------------------------------------------------------

function Update-BenchProject([string]$mainFw, [string]$bench, [string]$port) {
    $src = Join-Path $mainFw $projName
    $dst = Join-Path $bench $projName
    # Latin-1 maps bytes to chars one to one, so everything but the replaced text stays identical.
    $text = $latin1.GetString([System.IO.File]::ReadAllBytes($src))
    $from = '<OutputSubdirectory>Output\$(PlatformName.defuse)'
    $to = '<OutputSubdirectory>build\$(PlatformName.defuse)'
    if (-not $text.Contains($from)) {
        throw "OutputSubdirectory not found in $src; the script must be updated"
    }
    $text = $text.Replace($from, $to)
    if ($port) {
        # The port is patched in the bench copy only; the final text is what is compared below,
        # so an unchanged port does not rewrite the file.
        foreach ($tag in 'COMPort', 'ComPortName') {
            $pattern = "<$tag>[^<]*</$tag>"
            # A project without the port field is fine: the port is asked for when it is needed.
            if (-not [regex]::IsMatch($text, $pattern)) { continue }
            $text = [regex]::Replace($text, $pattern, "<$tag>$port</$tag>")
        }
    }
    $newBytes = $latin1.GetBytes($text)
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

function Initialize-BenchSource([string]$sourceRoot, [string]$mainRoot, [string]$bench) {
    $mainFw = Join-Path $mainRoot $fwRel
    $srcSketches = Join-Path (Join-Path $sourceRoot $fwRel) 'sketches'
    if (-not (Test-Path -LiteralPath $srcSketches -PathType Container)) {
        throw "No sketches folder at $srcSketches"
    }
    Update-BenchUserFile $mainFw $bench
    Set-Junction (Join-Path $bench 'Libs') (Join-Path $mainFw 'Libs')
    Set-Junction (Join-Path $bench 'sketches') $srcSketches
    Set-Content -LiteralPath (Join-Path $bench 'last-source.txt') -Value $sourceRoot -Encoding UTF8
}

# ---- Differences the bench build ignores --------------------------------------------------

# The project text without what does not change the build: the OtherFiles list (header names for
# the IDE only) and the COM port (patched per bench).
function Get-BuildRelevantProject([string]$path) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { return $null }
    $text = $latin1.GetString([System.IO.File]::ReadAllBytes($path)).Replace("`r`n", "`n")
    $text = [regex]::Replace($text, '<OtherFiles>.*?</OtherFiles>', '', [System.Text.RegularExpressions.RegexOptions]::Singleline)
    return [regex]::Replace($text, '<COMPort>[^<]*</COMPort>|<ComPortName>[^<]*</ComPortName>', '')
}

# HEAD of a submodule checkout, read from its .git file; a ref name is not resolved further.
function Get-SubmoduleHead([string]$dir) {
    $g = Join-Path $dir '.git'
    $gitDir = $null
    if (Test-Path -LiteralPath $g -PathType Leaf) {
        $line = Read-FirstLine $g
        if ($line -match '^gitdir:\s*(.+)$') {
            $gitDir = $Matches[1].Trim() -replace '/', '\'
            if (-not [System.IO.Path]::IsPathRooted($gitDir)) { $gitDir = Join-Path $dir $gitDir }
        }
    }
    elseif (Test-Path -LiteralPath $g -PathType Container) {
        $gitDir = $g
    }
    if (-not $gitDir) { return '' }
    $head = Read-FirstLine (Join-Path $gitDir 'HEAD')
    if ($head) { return $head }
    return ''
}

# One fingerprint per library folder: submodule HEAD plus a hash of relative file names and sizes.
# Modification times are left out on purpose: two checkouts never share them.
function Get-LibsFingerprint([string]$libsDir) {
    $result = @{}
    if (-not (Test-Path -LiteralPath $libsDir -PathType Container)) { return $result }
    $sha = [System.Security.Cryptography.SHA1]::Create()
    $root = New-Object System.IO.DirectoryInfo $libsDir
    foreach ($lib in $root.EnumerateDirectories()) {
        $prefix = $lib.FullName.Length
        $entries = New-Object 'System.Collections.Generic.List[string]'
        foreach ($f in $lib.EnumerateFiles('*', [System.IO.SearchOption]::AllDirectories)) {
            $rel = $f.FullName.Substring($prefix)
            if ($f.Name -eq '.git' -or $rel.Contains('\.git\')) { continue }
            $entries.Add($rel.ToLowerInvariant() + '|' + $f.Length)
        }
        $entries.Sort([System.StringComparer]::Ordinal)
        $bytes = [System.Text.Encoding]::UTF8.GetBytes([string]::Join("`n", $entries))
        $result[$lib.Name] = (Get-SubmoduleHead $lib.FullName) + '#' + $entries.Count + '#' + [System.BitConverter]::ToString($sha.ComputeHash($bytes))
    }
    return $result
}

# Warns, never stops: the bench builds the sketches of the chosen folder with the main clone's
# Libs and project, so a difference there silently gives a different firmware than that folder would.
function Test-SourceMatchesMain([string]$sourceRoot, [string]$mainRoot) {
    if (Test-SamePath $sourceRoot $mainRoot) { return }
    $srcFw = Join-Path $sourceRoot $fwRel
    $mainFw = Join-Path $mainRoot $fwRel

    $srcProj = Get-BuildRelevantProject (Join-Path $srcFw $projName)
    $mainProj = Get-BuildRelevantProject (Join-Path $mainFw $projName)
    if ($null -eq $srcProj) {
        Write-Warning "$sourceRoot has no $projName; the bench builds with the project of the main clone."
    }
    elseif ($srcProj -cne $mainProj) {
        Write-Warning "The VisualGDB project of $sourceRoot differs from the main clone's in a build-relevant way (board settings, flags or paths; a changed OtherFiles list does not count). The bench builds with the main clone's project."
    }

    $srcLibs = Get-LibsFingerprint (Join-Path $srcFw 'Libs')
    $mainLibs = Get-LibsFingerprint (Join-Path $mainFw 'Libs')
    $names = @($srcLibs.Keys) + @($mainLibs.Keys) | Sort-Object -Unique
    $different = @($names | Where-Object { $srcLibs[$_] -cne $mainLibs[$_] })
    if ($different.Count -gt 0) {
        Write-Warning ("Libs of $sourceRoot differ from the main clone's in: " + ($different -join ', ') + '. The bench builds with the main clone''s Libs.')
    }
}

# ---- Build --------------------------------------------------------------------------------

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
    # The log holds secret -D values (also in the command line of a failure report), so only
    # selected lines are shown; a line that looks like a command line is never echoed.
    $matched = @(Select-String -Path $log, $errLog -CaseSensitive -SimpleMatch -Pattern 'error:', 'Sketch uses', 'Global variables')
    foreach ($m in $matched) {
        $line = $m.Line
        if ($line.Contains('-D') -or $line.Contains('="') -or $line.Contains('Arguments') -or $line.Contains('-prefs=') -or ($line.Length -gt 300)) {
            Write-Host '<line suppressed: may contain build secrets>'
        }
        else {
            Write-Host $line
        }
    }
    Write-Host ('Build exit code {0}, {1:N1} s' -f $p.ExitCode, $watch.Elapsed.TotalSeconds)
    if ($p.ExitCode -ne 0 -and $matched.Count -eq 0) {
        Write-Host "Build failed, no error lines matched. Full log: $log (it contains build secrets - do not share it)"
    }
    return $p.ExitCode
}

# ---- Flash --------------------------------------------------------------------------------

function ConvertTo-VersionKey([string]$name) {
    try { return [version]$name } catch { return [version]'0.0' }
}

# The esp32 core folder and esptool.exe. The core is the one the last build compiled against
# (its folder is named in the dependency files of the sketch objects); without that evidence the
# highest installed version is used and a warning says so.
function Find-EspTools([string]$outDir) {
    $bases = @(
        (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'ArduinoData\packages\esp32'),
        (Join-Path $env:USERPROFILE 'Documents\ArduinoData\packages\esp32'),
        (Join-Path $env:LOCALAPPDATA 'Arduino15\packages\esp32')
    )
    $roots = @()
    foreach ($b in $bases) {
        if ($b -and (Test-Path -LiteralPath $b -PathType Container) -and -not ($roots | Where-Object { Test-SamePath $_ $b })) { $roots += $b }
    }
    $cores = @()
    foreach ($r in $roots) {
        $hw = Join-Path $r 'hardware\esp32'
        if (Test-Path -LiteralPath $hw -PathType Container) {
            foreach ($d in (Get-ChildItem -LiteralPath $hw -Directory)) {
                $cores += [pscustomobject]@{ Root = $r; Version = $d.Name; Path = $d.FullName }
            }
        }
    }
    if ($cores.Count -eq 0) { throw 'No esp32 core found under ArduinoData\packages or Arduino15\packages.' }

    $usedVersion = $null
    $depFile = $null
    $sketchOut = Join-Path $outDir 'sketch'
    if (Test-Path -LiteralPath $sketchOut -PathType Container) {
        $depFile = Get-ChildItem -LiteralPath $sketchOut -Filter '*.d' -File | Select-Object -First 1
    }
    if ($depFile) {
        $m = [regex]::Match([System.IO.File]::ReadAllText($depFile.FullName), 'hardware[\\/]+esp32[\\/]+(\d[^\\/\s]*)')
        if ($m.Success) { $usedVersion = $m.Groups[1].Value }
    }
    $core = $null
    if ($usedVersion) { $core = $cores | Where-Object { $_.Version -eq $usedVersion } | Select-Object -First 1 }
    if (-not $core) {
        $core = $cores | Sort-Object { ConvertTo-VersionKey $_.Version } -Descending | Select-Object -First 1
        if ($cores.Count -gt 1 -or $usedVersion) {
            Write-Warning "Could not tell which esp32 core the build used; flashing with the highest installed one ($($core.Version))."
        }
    }

    $tools = @()
    foreach ($r in (@($core.Root) + @($roots | Where-Object { -not (Test-SamePath $_ $core.Root) }))) {
        $tp = Join-Path $r 'tools\esptool_py'
        if (-not (Test-Path -LiteralPath $tp -PathType Container)) { continue }
        foreach ($d in (Get-ChildItem -LiteralPath $tp -Directory)) {
            $exe = Join-Path $d.FullName 'esptool.exe'
            if (Test-Path -LiteralPath $exe -PathType Leaf) { $tools += [pscustomobject]@{ Version = $d.Name; Exe = $exe } }
        }
        if ($tools.Count -gt 0) { break }
    }
    if ($tools.Count -eq 0) { throw 'esptool.exe not found under packages\esp32\tools\esptool_py.' }
    $esptool = ($tools | Sort-Object { ConvertTo-VersionKey $_.Version } -Descending | Select-Object -First 1).Exe
    return [pscustomobject]@{ CorePath = $core.Path; CoreVersion = $core.Version; EsptoolExe = $esptool }
}

function Get-ProjectProperty([string]$text, [string]$key) {
    $m = [regex]::Match($text, "<Key>$key</Key>\s*<Value>([^<]*)</Value>")
    if (-not $m.Success) { throw "Board property $key not found in the bench project; set it in the VisualGDB project properties." }
    return $m.Groups[1].Value.Trim()
}

# esp32.menu.<menu>.<choice>.<property>=<value> from boards.txt.
function Get-BoardMenuValue([string]$boardsFile, [string]$menu, [string]$choice, [string]$property) {
    $prefix = "esp32.menu.$menu.$choice.$property="
    foreach ($line in [System.IO.File]::ReadLines($boardsFile)) {
        if ($line.StartsWith($prefix, [System.StringComparison]::Ordinal)) { return $line.Substring($prefix.Length).Trim() }
    }
    throw "boards.txt has no $prefix entry; the board property '$menu' = '$choice' is not supported by the script."
}

function Format-CommandLine([string]$exe, [string[]]$arguments) {
    $parts = @('"' + $exe + '"')
    foreach ($a in $arguments) {
        if ($a -match '\s') { $parts += ('"' + $a + '"') } else { $parts += $a }
    }
    return ($parts -join ' ')
}

function Stop-OldMonitor([string]$bench) {
    $pidFile = Join-Path $bench 'monitor.pid'
    if (-not (Test-Path -LiteralPath $pidFile -PathType Leaf)) { return }
    $lines = @(Get-Content -LiteralPath $pidFile -Encoding ASCII)
    $id = 0
    $ticks = [long]0
    $valid = ($lines.Count -ge 2) -and [int]::TryParse($lines[0], [ref]$id) -and [long]::TryParse($lines[1], [ref]$ticks)
    if ($valid) {
        $proc = Get-Process -Id $id -ErrorAction SilentlyContinue
        $same = $false
        try {
            # The start time tells the monitor from an unrelated process that got the PID later.
            $same = $proc -and ($proc.ProcessName -match '^pythonw?$') -and ($proc.StartTime.ToUniversalTime().Ticks -eq $ticks)
        }
        catch {
            $same = $false
        }
        if ($same) {
            if ($DryRun) {
                Write-Host "Would stop the monitor started earlier (PID $id)."
                return
            }
            Write-Host "Stopping the monitor started earlier (PID $id)."
            Stop-Process -Id $id -Force
            Start-Sleep -Milliseconds 500
        }
    }
    if (-not $DryRun) { [System.IO.File]::Delete($pidFile) }
}

# $notOlderThan: when set (Build + Flash), the app image must have been written after that time,
# otherwise the build did not produce a new image and an old one would be flashed.
function Invoke-Flash([string]$bench, [string]$cfg, [string]$port, [Nullable[datetime]]$notOlderThan = $null) {
    if (-not $port) { throw 'No COM port is set. Run the Change COM port task.' }
    $projFile = Join-Path $bench $projName
    if (-not (Test-Path -LiteralPath $projFile -PathType Leaf)) { throw 'The bench has no project yet. Build first.' }
    $text = $latin1.GetString([System.IO.File]::ReadAllBytes($projFile))
    $outDir = Join-Path $bench "build\$platform\$cfg"
    $sketchFile = [regex]::Match($text, '<MainSketchFile>([^<]+)</MainSketchFile>').Groups[1].Value
    if (-not $sketchFile) { throw "MainSketchFile not found in $projFile" }

    $bootloader = Join-Path $outDir "$sketchFile.bootloader.bin"
    $partitions = Join-Path $outDir "$sketchFile.partitions.bin"
    $app = Join-Path $outDir "$sketchFile.bin"
    foreach ($f in $bootloader, $partitions, $app) {
        if (-not (Test-Path -LiteralPath $f -PathType Leaf)) {
            throw "Nothing to flash: $f is missing. Build first (task Build, or Build + Flash + Monitor)."
        }
    }

    if ($null -ne $notOlderThan -and (Get-Item -LiteralPath $app).LastWriteTime -lt $notOlderThan) {
        throw 'The build did not produce a new image, not flashing.'
    }

    $tools = Find-EspTools $outDir
    $bootApp = Join-Path $tools.CorePath 'tools\partitions\boot_app0.bin'
    $boards = Join-Path $tools.CorePath 'boards.txt'
    foreach ($f in $bootApp, $boards) {
        if (-not (Test-Path -LiteralPath $f -PathType Leaf)) { throw "$f is missing; the esp32 core looks incomplete." }
    }

    $uploadSpeed = Get-BoardMenuValue $boards 'UploadSpeed' (Get-ProjectProperty $text 'UploadSpeed') 'upload.speed'
    $flashMode = Get-BoardMenuValue $boards 'FlashMode' (Get-ProjectProperty $text 'FlashMode') 'build.flash_mode'
    $flashFreq = Get-BoardMenuValue $boards 'FlashFreq' (Get-ProjectProperty $text 'FlashFreq') 'build.flash_freq'
    $flashSize = Get-BoardMenuValue $boards 'FlashSize' (Get-ProjectProperty $text 'FlashSize') 'build.flash_size'

    $flashArgs = @(
        '--chip', 'esp32', '--port', $port, '--baud', $uploadSpeed,
        '--before', 'default_reset', '--after', 'hard_reset',
        'write_flash', '-z', '--flash_mode', $flashMode, '--flash_freq', $flashFreq, '--flash_size', $flashSize,
        '0x1000', $bootloader, '0x8000', $partitions, '0xe000', $bootApp, '0x10000', $app
    )

    $lastSource = Read-FirstLine (Join-Path $bench 'last-source.txt')
    $built = (Get-Item -LiteralPath $app).LastWriteTime
    Write-Host ("Flashing the build of {0} ({1:yyyy-MM-dd HH:mm:ss}) to {2} at {3} baud, esp32 core {4}." -f $lastSource, $built, $port, $uploadSpeed, $tools.CoreVersion)

    if ($DryRun) {
        Write-Host (Format-CommandLine $tools.EsptoolExe $flashArgs)
        return 0
    }
    # Start-Process instead of "&": the child writes straight to the console (live progress) and
    # nothing leaks into this function's return value.
    $argLine = (Format-CommandLine '' $flashArgs).Substring(3)
    $p = Start-Process -FilePath $tools.EsptoolExe -ArgumentList $argLine -NoNewWindow -Wait -PassThru
    $code = $p.ExitCode
    if ($code -ne 0) {
        Write-Host ''
        Write-Host "esptool failed (exit code $code). If it could not open ${port}: the port is busy (is a terminal or monitor still open on it, for example in Visual Studio?) or it is the wrong port. Pick another with the Change COM port task. If it could not connect: hold BOOT while it starts."
    }
    else {
        Write-Host 'Flash done, the lamp restarts.'
    }
    return $code
}

# ---- Monitor ------------------------------------------------------------------------------

# Python that has pyserial: returns @{ Ok; Python; Message }.
function Test-MonitorPython {
    if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
        return [pscustomobject]@{ Ok = $false; Python = $null; Message = "The Python launcher 'py' is not found. Install Python 3, then: py -3 -m pip install pyserial" }
    }
    $prev = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $out = & py -3 -c 'import sys, serial.tools.miniterm; print(sys.executable)' 2>&1
        $exit = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $prev
    }
    $outText = ($out | Out-String).Trim()
    if ($exit -ne 0) {
        if ($outText -match 'No module named') {
            return [pscustomobject]@{ Ok = $false; Python = $null; Message = 'pyserial is not installed. Install it with: py -3 -m pip install pyserial' }
        }
        return [pscustomobject]@{ Ok = $false; Python = $null; Message = "Python could not start the monitor: $outText" }
    }
    return [pscustomobject]@{ Ok = $true; Python = ($outText -split "`r?`n" | Select-Object -Last 1).Trim(); Message = '' }
}

function Invoke-Monitor([string]$bench, [string]$port) {
    if (-not $port) { throw 'No COM port is set. Run the Change COM port task.' }
    $projFile = Join-Path $bench $projName
    if (-not (Test-Path -LiteralPath $projFile -PathType Leaf)) { throw 'The bench has no project yet. Run a build or the Change COM port task first.' }
    $text = $latin1.GetString([System.IO.File]::ReadAllBytes($projFile))
    # DeviceTerminalSettings: the baud rate VisualGDB's terminal uses for this lamp.
    $m = [regex]::Match($text, '<BaudRate>(\d+)</BaudRate>')
    if (-not $m.Success) { throw 'BaudRate not found in the bench project.' }
    $baud = $m.Groups[1].Value

    # DTR/RTS stay low: raising them on open would reset the lamp through the USB-UART auto-reset
    # circuit. LF-only lines need no option: the Windows console returns to column 0 on LF.
    $miniArgs = @('-m', 'serial.tools.miniterm', '--dtr', '0', '--rts', '0', '--eol', 'LF', $port, $baud)
    $check = Test-MonitorPython

    if ($DryRun) {
        Write-Host ('py -3 ' + ($miniArgs -join ' '))
        if (-not $check.Ok) { Write-Warning $check.Message }
        return 0
    }
    if (-not $check.Ok) { throw $check.Message }

    Write-Host "Monitor on $port at $baud baud. Exit with Ctrl+]."
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $check.Python
    $psi.Arguments = ($miniArgs -join ' ')
    $psi.UseShellExecute = $false
    $proc = [System.Diagnostics.Process]::Start($psi)
    $pidFile = Join-Path $bench 'monitor.pid'
    Set-Content -LiteralPath $pidFile -Value @([string]$proc.Id, [string]$proc.StartTime.ToUniversalTime().Ticks) -Encoding ASCII
    try {
        $proc.WaitForExit()
    }
    finally {
        $line = Read-FirstLine $pidFile
        if ($line -eq [string]$proc.Id) { [System.IO.File]::Delete($pidFile) }
    }
    if ($proc.ExitCode -ne 0) {
        Write-Host "The monitor ended with exit code $($proc.ExitCode). If it could not open ${port}: the port is busy or wrong; pick another with the Change COM port task."
    }
    return $proc.ExitCode
}

# ---- Main ---------------------------------------------------------------------------------

function Invoke-Main {
    $mainRoot = Get-MainCloneRoot
    $mainFw = Join-Path $mainRoot $fwRel
    $bench = $BenchRoot
    if (-not $bench) { $bench = Join-Path $mainRoot '.bench' }
    # Full path before any use: .NET and child processes resolve relative paths against another directory.
    if (-not [System.IO.Path]::IsPathRooted($bench)) { $bench = Join-Path (Get-Location).ProviderPath $bench }
    $bench = [System.IO.Path]::GetFullPath($bench).TrimEnd('\')

    $needBuild = $Action -in 'Build', 'BuildFlashMonitor'
    $needFlash = $Action -in 'Flash', 'FlashMonitor', 'BuildFlashMonitor'
    $needMonitor = $Action -in 'Monitor', 'FlashMonitor', 'BuildFlashMonitor'

    New-Item -ItemType Directory -Force -Path $bench, (Join-Path $bench 'build') | Out-Null

    $sourceRoot = $null
    if ($needBuild) {
        if ($Source) { $sourceRoot = (Resolve-Path -LiteralPath $Source).Path.TrimEnd('\') }
        else { $sourceRoot = Select-Source $mainRoot $bench }
    }

    # Build only patches the stored port; every other action may ask about it.
    $comPort = Resolve-Port $mainFw $bench $Port ($Action -ne 'Build') ($Action -eq 'Port')
    Update-BenchProject $mainFw $bench $comPort

    if ($Action -eq 'Port') {
        Write-Host "COM port: $comPort (stored in $(Join-Path $bench 'port'))"
        return 0
    }

    $buildStart = $null
    if ($needBuild) {
        Test-SourceMatchesMain $sourceRoot $mainRoot
        Initialize-BenchSource $sourceRoot $mainRoot $bench
        $buildStart = Get-Date
        $code = Invoke-Build $bench $Config
        if ($code -ne 0) { return $code }
    }
    if ($needFlash) {
        Stop-OldMonitor $bench
        $code = Invoke-Flash $bench $Config $comPort $buildStart
        if ($code -ne 0) { return $code }
    }
    if ($needMonitor) {
        [void](Invoke-Monitor $bench $comPort)
    }
    return 0
}

try {
    exit (Invoke-Main)
}
catch {
    Write-Host ("ERROR: " + $_.Exception.Message) -ForegroundColor Red
    exit 1
}
