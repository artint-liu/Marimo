# set_env.ps1
# Get script directory (with trailing backslash)
$marimoDir = $PSScriptRoot + "\"
$clstdDir  = $PSScriptRoot + "\clstd\"

# Write to system environment variables (requires Administrator)
[System.Environment]::SetEnvironmentVariable("MARIMO_DIR", $marimoDir, "User")
[System.Environment]::SetEnvironmentVariable("CLSTD_DIR",  $clstdDir,  "User")

Write-Host "=============================="
Write-Host "Done!"
Write-Host ("MARIMO_DIR = " + [System.Environment]::GetEnvironmentVariable("MARIMO_DIR", "User"))
Write-Host ("CLSTD_DIR  = " + [System.Environment]::GetEnvironmentVariable("CLSTD_DIR",  "User"))
Write-Host "=============================="
Read-Host "Press Enter to exit"
