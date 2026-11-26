[CmdletBinding()]
param(
    [string]$SourceDir = (Split-Path -Parent $PSScriptRoot),
    [ValidatePattern('^[0-9a-fA-F]{40}([0-9a-fA-F]{24})?$')][string]$SourceRevision,
    [ValidatePattern('^[0-9a-fA-F]{40}([0-9a-fA-F]{24})?$')][string]$SourceTree,
    [string]$BuildRoot,
    [string]$CMake = 'cmake',
    [string]$CTest = 'ctest',
    [string]$Generator = 'Ninja',
    [string]$Compiler,
    [string]$CompilerLauncher,
    [string]$Ninja,
    [string]$GTestPrefix,
    [ValidateNotNullOrEmpty()][ValidateSet('Debug', 'Release')][string[]]$Configuration = @('Debug', 'Release'),
    [ValidateRange(1, 1024)][int]$Parallel = 2,
    [switch]$Sanitizers
)

# PowerShell 5.1 and pwsh: every native exit code is checked explicitly. Evidence
# belongs to the tested worktree, never to another checkout's build directory.
$ErrorActionPreference = 'Stop'
$SourceDir = (Resolve-Path -LiteralPath $SourceDir).Path
if (!$BuildRoot) { $BuildRoot = Join-Path $SourceDir 'build/verify' }
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null
$run = Join-Path $BuildRoot ('evidence-' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ') + '-' + [Guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $run | Out-Null
$overallClock = [Diagnostics.Stopwatch]::StartNew()
$commands = New-Object System.Collections.ArrayList
$results = New-Object System.Collections.ArrayList
$evidence = [ordered]@{
    startedUtc = [DateTime]::UtcNow.ToString('o')
    completedUtc = $null
    elapsedSeconds = $null
    # A configured launcher is not evidence of cache hits. No cache statistics
    # are collected here; never infer a warm cache from its executable path.
    compilerLauncher = [ordered]@{
        requested = $CompilerLauncher; resolved = $null; sha256 = $null
        version = 'unknown'; cacheState = 'unknown'
    }
    settings = [ordered]@{
        configurations = @($Configuration); generator = $Generator; compiler = $Compiler
        ninja = $Ninja; gtestPrefix = $GTestPrefix; parallel = $Parallel; sanitizers = [bool]$Sanitizers
    }
    source = $SourceDir
    sourceKind = $null
    sourceRevision = $null
    sourceTree = $null
    head = $null
    dirtyPaths = @()
    configurations = $results
    commands = $commands
    success = $false
    error = $null
}

function Invoke-Checked([string]$Executable, [string[]]$Arguments, [string]$LogName,
                        [string]$Phase = 'provenance', [string]$ConfigurationTag = '') {
    $log = Join-Path $run $LogName
    $clock = [Diagnostics.Stopwatch]::StartNew()
    $entry = [ordered]@{ executable = $Executable; arguments = $Arguments; log = $log; exitCode = $null
        phase = $Phase; configuration = $ConfigurationTag; startedUtc = [DateTime]::UtcNow.ToString('o')
        completedUtc = $null; elapsedSeconds = $null; error = $null }
    [void]$commands.Add($entry)
    try {
        # Resolution belongs inside the timed failure boundary.
        $resolved = Get-Command $Executable -ErrorAction Stop
        $savedPreference = $ErrorActionPreference
        try {
            $ErrorActionPreference = 'Continue'
            $output = @(& $resolved @Arguments 2>&1)
            $code = $LASTEXITCODE
        } finally { $ErrorActionPreference = $savedPreference }
        $entry.exitCode = $code
        $text = ($output | ForEach-Object { $_.ToString() }) -join "`n"
        [IO.File]::WriteAllText($log, $text + "`n", (New-Object Text.UTF8Encoding($false)))
        if ($code -ne 0) { throw "Native command failed ($code): $Executable; see $log" }
        return $text
    } catch {
        $entry.error = $_.Exception.Message
        throw
    } finally {
        $clock.Stop()
        $entry.completedUtc = [DateTime]::UtcNow.ToString('o')
        $entry.elapsedSeconds = $clock.Elapsed.TotalSeconds
    }
}

try {
    if (Test-Path -LiteralPath (Join-Path $SourceDir '.git')) {
        if ($SourceRevision -or $SourceTree) { throw 'Do not override provenance for a Git working-tree root' }
        $evidence.sourceKind = 'working-tree'
        $top = (Invoke-Checked 'git' @('-C', $SourceDir, 'rev-parse', '--show-toplevel') 'git-root.log').Trim()
        if ([IO.Path]::GetFullPath($top) -ne $SourceDir) { throw 'SourceDir must be the exact Git working-tree root' }
        $evidence.head = (Invoke-Checked 'git' @('-C', $SourceDir, 'rev-parse', 'HEAD') 'git-head.log').Trim()
        $evidence.sourceRevision = $evidence.head
        $dirty = Invoke-Checked 'git' @('-C', $SourceDir, 'status', '--short', '--untracked-files=all') 'git-status.log'
        $evidence.dirtyPaths = @($dirty -split "`n" | Where-Object { $_ })
    } else {
        # Exports inside build/ must never inherit their containing checkout's
        # HEAD/status. The caller supplies provenance from the validated index.
        if (!$SourceRevision -or !$SourceTree) { throw 'Non-Git exports require both SourceRevision and SourceTree provenance' }
        $evidence.sourceKind = 'staged-export'
        $evidence.sourceRevision = $SourceRevision
        $evidence.sourceTree = $SourceTree
    }
    if ($CompilerLauncher) {
        $launcherCommand = Get-Command $CompilerLauncher -CommandType Application -ErrorAction Stop
        $evidence.compilerLauncher.resolved = $launcherCommand.Path
        if (!$evidence.compilerLauncher.resolved) { throw 'Compiler launcher must resolve to an executable file' }
        # Record the resolved bytes, not only a mutable path. Hash failure must
        # stop verification. Arbitrary wrappers need not support --version.
        $evidence.compilerLauncher.sha256 = (Get-FileHash -LiteralPath $launcherCommand.Path -Algorithm SHA256 -ErrorAction Stop).Hash
    }
    Invoke-Checked $CMake @('--version') 'cmake-version.log' 'toolchain' | Out-Null
    Invoke-Checked $CTest @('--version') 'ctest-version.log' 'toolchain' | Out-Null
    foreach ($config in $Configuration) {
        $build = Join-Path (Join-Path $run 'build') $config.ToLowerInvariant()
        # CMake >=3.24 maps this to /WX on MSVC and -Werror on GCC/Clang.
        $args = @('-S', $SourceDir, '-B', $build, '-G', $Generator,
            "-DCMAKE_BUILD_TYPE=$config", '-DBUILD_TESTING=ON', '-DCMAKE_COMPILE_WARNING_AS_ERROR=ON')
        if ($CompilerLauncher) { $args += "-DCMAKE_CXX_COMPILER_LAUNCHER=$($evidence.compilerLauncher.resolved)" }
        if ($Compiler) { $args += "-DCMAKE_CXX_COMPILER=$Compiler" }
        if ($Ninja) { $args += "-DCMAKE_MAKE_PROGRAM=$Ninja" }
        if ($GTestPrefix) { $args += "-DCMAKE_PREFIX_PATH=$GTestPrefix" }
        $sanitizerMode = if ($Sanitizers) { 'ON' } else { 'OFF' }
        $args += "-DEGALIS_ENABLE_SANITIZERS=$sanitizerMode"
        Invoke-Checked $CMake $args "$config-configure.log" 'configure' $config | Out-Null
        Invoke-Checked $CMake @('--build', $build, '--config', $config, '--parallel', "$Parallel") "$config-build.log" 'build' $config | Out-Null
        $inventory = Invoke-Checked $CTest @('--test-dir', $build, '-C', $config, '--show-only=json-v1') "$config-inventory.json" 'inventory' $config
        $discovered = @((ConvertFrom-Json $inventory).tests)
        $count = $discovered.Count
        if ($count -eq 0) { throw "No tests discovered in $config" }
        $junit = Join-Path $run "$config-tests.xml"
        Invoke-Checked $CTest @('--test-dir', $build, '-C', $config, '--output-on-failure', '--no-tests=error', '--output-junit', $junit) "$config-tests.log" 'tests' $config | Out-Null
        [xml]$report = Get-Content -LiteralPath $junit -Raw
        if ($report.SelectNodes('//testcase').Count -ne $count) { throw "JUnit count does not match discovered tests in $config" }
        # Equal counts do not prove every discovered test ran: substitutions or
        # duplicate results can conceal a missing test. Compare ordinal multisets.
        [string[]]$expectedNames = @($discovered | ForEach-Object { $_.name })
        [string[]]$actualNames = @($report.SelectNodes('//testcase') | ForEach-Object { $_.GetAttribute('name') })
        [Array]::Sort($expectedNames, [StringComparer]::Ordinal)
        [Array]::Sort($actualNames, [StringComparer]::Ordinal)
        for ($i = 0; $i -lt $count; $i++) {
            if (![String]::Equals($expectedNames[$i], $actualNames[$i], [StringComparison]::Ordinal)) {
                throw "JUnit identities do not match discovered tests in $config"
            }
        }
        if ($report.SelectNodes('//testcase/failure | //testcase/error').Count -gt 0) { throw "JUnit reports failures in $config" }
        if ($report.SelectNodes('//testcase/skipped').Count -gt 0) { throw "Skipped tests in $config; full verification is incomplete" }
        [void]$results.Add([ordered]@{ configuration = $config; tests = $count; passed = $true; build = $build; junit = $junit })
        Write-Output "$config PASS: $count tests; warnings as errors"
    }
    $evidence.success = $true
} catch {
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $overallClock.Stop()
    $evidence.elapsedSeconds = $overallClock.Elapsed.TotalSeconds
    $evidence.completedUtc = [DateTime]::UtcNow.ToString('o')
    $json = $evidence | ConvertTo-Json -Depth 8
    [IO.File]::WriteAllText((Join-Path $run 'verification.json'), $json + "`n", (New-Object Text.UTF8Encoding($false)))
    Write-Output "Verification evidence: $run"
}
