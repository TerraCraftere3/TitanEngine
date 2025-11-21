function Remove-GeneratedFiles {
    param (
        [string]$Path
    )

    $pattern = "*.generated.*"

    $files = Get-ChildItem -Path $Path -Recurse -File -Filter $pattern

    foreach ($file in $files) {
        Write-Host "$($file.FullName)" -ForegroundColor Yellow
        Remove-Item $file.FullName -Force
    }
}

Write-Host "Removing generated files..." -ForegroundColor Cyan

Remove-GeneratedFiles ".\Engine"
Remove-GeneratedFiles ".\Script-Core"
Remove-GeneratedFiles ".\Editor"
Remove-GeneratedFiles ".\Sandbox"
Remove-GeneratedFiles ".\Runtime"

Write-Host "Done!" -ForegroundColor Cyan
