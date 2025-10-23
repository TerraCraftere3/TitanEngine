function Format-FilesInDirectory {
    param (
        [string]$Path
    )

    $extensions = "*.cpp","*.c","*.cc","*.cxx","*.h","*.hpp","*.hxx","*.inl","*.glsl","*.vert","*.frag","*.comp","*.geom","*.tesc","*.tese","*.proto"

    Get-ChildItem -Path $Path -Recurse -Include $extensions | ForEach-Object {
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

Format-FilesInDirectory -Path ".\Engine"
Format-FilesInDirectory -Path ".\Editor"
Format-FilesInDirectory -Path ".\Sandbox"
Format-FilesInDirectory -Path ".\Runtime"

Write-Host "Finished Formatting!" -ForegroundColor Cyan
