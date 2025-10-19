function Format-FilesInDirectory {
    param (
        [string]$Path
    )

    Get-ChildItem -Path $Path -Recurse -Include *.cpp,*.h | ForEach-Object {
        $original = Get-Content $_.FullName -Raw
        clang-format -i $_.FullName
        $formatted = Get-Content $_.FullName -Raw

        if ($original -ne $formatted) {
            Write-Host "$($_.FullName)" -ForegroundColor Green
        } else {
            Write-Host "$($_.FullName)" -ForegroundColor Gray
        }
    }
}

Write-Host "Starting Formatting..." -ForegroundColor Cyan

Format-FilesInDirectory -Path ".\Engine\src"
Format-FilesInDirectory -Path ".\Sandbox\src"

Write-Host "Finished Formatting!" -ForegroundColor Cyan
