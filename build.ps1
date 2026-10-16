param (
    [Parameter(Mandatory=$true)]
    [string]$SourceFile
)

$vcvarsPath = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
$compileCmd = "cl /EHsc $SourceFile"

$fullCmd = "`"$vcvarsPath`" && $compileCmd"

Write-Host "Building $SourceFile..."
cmd /c $fullCmd

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Build successful."
