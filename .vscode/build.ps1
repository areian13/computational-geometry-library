param(
    [ValidateSet('Build', 'Rebuild', 'Clean', 'Configure')]
    [string]$Action = 'Build',
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$projectPath = Join-Path $repoRoot 'computational-geometry-library.vcxproj'
$solutionPath = Join-Path $repoRoot 'computational-geometry-library.sln'

try {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'Install Visual Studio 2022 (17.13 or newer) with Desktop development with C++ and a Windows SDK.'
    }

    $vsInstall = & $vswhere -latest -products '*' -version '[17.13,)' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $vsInstall) {
        throw 'No compatible MSVC installation found. Install the Visual Studio C++ workload.'
    }
    $msbuild = Join-Path $vsInstall 'MSBuild\Current\Bin\MSBuild.exe'
    if (-not (Test-Path -LiteralPath $msbuild)) {
        throw "MSBuild not found: $msbuild"
    }

    $configurations = foreach ($config in @('Debug', 'Release')) {
        $projectJson = & $msbuild $projectPath /nologo "/p:Configuration=$config" /p:Platform=x64 `
            '-getProperty:VCToolsInstallDir,IncludePath' '-getItem:ClCompile'
        if ($LASTEXITCODE -ne 0) {
            throw "Cannot read $config project settings from MSBuild."
        }
        $projectData = ($projectJson | Out-String) | ConvertFrom-Json
        $source = $projectData.Items.ClCompile | Where-Object { $_.Identity -eq 'main.cpp' } | Select-Object -First 1
        if (-not $source) {
            throw 'main.cpp must be registered as ClCompile in the Visual Studio project.'
        }
        $compiler = Join-Path $projectData.Properties.VCToolsInstallDir 'bin\Hostx64\x64\cl.exe'
        if (-not (Test-Path -LiteralPath $compiler)) {
            throw "MSVC compiler not found: $compiler"
        }
        $cppStandard = switch ($source.LanguageStandard) {
            'stdcpp20' { 'c++20' }
            'stdcpp23' { 'c++23' }
            'stdcpplatest' { 'c++23' }
            default { throw "Unsupported C++ standard: $($source.LanguageStandard). Use C++20 or C++23." }
        }
        $includes = @('${workspaceFolder}')
        $includeDirectories = "$($source.AdditionalIncludeDirectories);$($projectData.Properties.IncludePath)"
        foreach ($include in ($includeDirectories -split ';' | Where-Object { $_ })) {
            if ([IO.Path]::IsPathRooted($include)) {
                $includes += $include
            } else {
                $includes += [IO.Path]::GetFullPath((Join-Path $repoRoot $include))
            }
        }
        [ordered]@{
            name = "MSVC x64 $config"
            compilerPath = $compiler
            compilerArgs = @($source.AdditionalOptions -split '\s+' | Where-Object { $_ })
            intelliSenseMode = 'windows-msvc-x64'
            cppStandard = $cppStandard
            includePath = @($includes | Select-Object -Unique)
            defines = @($source.PreprocessorDefinitions -split ';' | Where-Object { $_ })
        }
    }
    $intelliSensePath = Join-Path $PSScriptRoot 'c_cpp_properties.json'
    $intelliSenseJson = [ordered]@{ configurations = @($configurations); version = 4 } | ConvertTo-Json -Depth 6
    if (-not (Test-Path -LiteralPath $intelliSensePath) -or
        [IO.File]::ReadAllText($intelliSensePath).Trim() -ne $intelliSenseJson.Trim()) {
        [IO.File]::WriteAllText($intelliSensePath, $intelliSenseJson + [Environment]::NewLine, [Text.UTF8Encoding]::new($false))
    }

    if ($Action -eq 'Configure') {
        Write-Host 'IntelliSense configured for MSVC x64 Debug / Release.'
        exit 0
    }

    Write-Host "MSBuild: $msbuild"
    & $msbuild $solutionPath /nologo /m /v:minimal "/t:$Action" "/p:Configuration=$Configuration" /p:Platform=x64
    exit $LASTEXITCODE
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
