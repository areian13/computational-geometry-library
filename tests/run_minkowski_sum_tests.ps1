param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64'
)

& (Join-Path $PSScriptRoot 'run_tests.ps1') -Suite MinkowskiSum -Configuration $Configuration -Platform $Platform
exit $LASTEXITCODE
