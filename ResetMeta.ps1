Write-Host "Searching for *.meta files in '$Path'..." -ForegroundColor Cyan

# Find all *.meta files recursively
$metaFiles = Get-ChildItem -Path $Path -Recurse -Filter "*.meta" -File -ErrorAction SilentlyContinue

if ($metaFiles.Count -eq 0) {
    Write-Host "No .meta files found." -ForegroundColor Yellow
    exit
}

Write-Host "Found $($metaFiles.Count) .meta files. Removing..." -ForegroundColor Green

foreach ($file in $metaFiles) {
    try {
        Remove-Item $file.FullName -Force
        Write-Host "Deleted: $($file.FullName)" -ForegroundColor DarkGray
    }
    catch {
        Write-Host "Failed to delete: $($file.FullName)" -ForegroundColor Red
    }
}

Write-Host "All .meta files removed." -ForegroundColor Green