param(
    [ValidateSet('DCEL', 'Triangulation', 'PolygonIntersection', 'MinkowskiSum')]
    [string]$Suite = 'DCEL',
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$previousInclude = $env:INCLUDE
$previousLib = $env:LIB
try {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $vsInstall = & $vswhere -latest -products '*' -version '[17.13,)' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $vsInstall) { throw 'No compatible MSVC installation found.' }
    $msbuild = Join-Path $vsInstall 'MSBuild\Current\Bin\MSBuild.exe'
    $projectPath = Join-Path $repoRoot 'computational-geometry-library.vcxproj'
    $projectJson = & $msbuild $projectPath /nologo "/p:Configuration=$Configuration" "/p:Platform=$Platform" `
        '-getProperty:VCToolsInstallDir,IncludePath,LibraryPath' '-getItem:ClCompile'
    if ($LASTEXITCODE -ne 0) { throw 'Cannot read the Visual Studio compiler settings.' }
    $data = ($projectJson | Out-String) | ConvertFrom-Json
    $source = $data.Items.ClCompile | Where-Object { $_.Identity -eq 'main.cpp' } | Select-Object -First 1
    $standard = switch ($source.LanguageStandard) {
        'stdcpp20' { '/std:c++20' }
        'stdcpp23' { '/std:c++23preview' }
        'stdcpplatest' { '/std:c++latest' }
        default { throw "Unsupported standard: $($source.LanguageStandard)" }
    }
    $architecture = if ($Platform -eq 'x64') { 'x64' } else { 'x86' }
    $compiler = Join-Path $data.Properties.VCToolsInstallDir "bin\Hostx64\$architecture\cl.exe"
    $outputDir = Join-Path $repoRoot "obj\tests\$Platform\$Configuration"
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    $testName = switch ($Suite) {
        'PolygonIntersection' { 'polygon_intersection_tests' }
        'MinkowskiSum' { 'minkowski_sum_tests' }
        default { $Suite.ToLowerInvariant() + '_tests' }
    }
    $program = Join-Path $outputDir ($testName + '.exe')
    $env:INCLUDE = $data.Properties.IncludePath
    $env:LIB = $data.Properties.LibraryPath
    $compilerArgs = @('/nologo', '/EHsc', '/W4', '/permissive-', $standard)
    $compilerArgs += @($source.AdditionalOptions -split '\s+' | Where-Object { $_ })
    foreach ($define in ($source.PreprocessorDefinitions -split ';' | Where-Object { $_ })) { $compilerArgs += "/D$define" }
    if ($Configuration -eq 'Debug') { $compilerArgs += @('/MDd', '/Od', '/Zi') }
    else { $compilerArgs += @('/MD', '/O2') }
    $compilerArgs += @("/Fo$(Join-Path $outputDir ($testName + '.obj'))", "/Fd$(Join-Path $outputDir ($testName + '_compile.pdb'))",
        "/Fe$program", (Join-Path $PSScriptRoot ($testName + '.cpp')), '/link', "/PDB:$(Join-Path $outputDir ($testName + '.pdb'))")
    & $compiler @compilerArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $program
    exit $LASTEXITCODE
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
} finally {
    $env:INCLUDE = $previousInclude
    $env:LIB = $previousLib
}
