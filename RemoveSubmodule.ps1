param (
    [Parameter(Mandatory = $true)]
    [string]$SubmodulePath
)

# Exit on errors
$ErrorActionPreference = "Stop"

Write-Host "Removing submodule '$SubmodulePath'..."

# Normalize path slashes
$SubmodulePath = $SubmodulePath -replace '\\', '/'

# Ensure we're in a Git repo
if (-not (Test-Path ".git")) {
    Write-Error "This script must be run from the root of a Git repository."
    exit 1
}

# Step 1: Deinitialize the submodule
Write-Host "→ Deinitializing submodule..."
git submodule deinit -f $SubmodulePath 2>$null

# Step 2: Remove from index
Write-Host "→ Removing submodule from index..."
git rm -f $SubmodulePath 2>$null

# Step 3: Remove section from .gitmodules
if (Test-Path ".gitmodules") {
    Write-Host "→ Removing entry from .gitmodules..."
    git config -f .gitmodules --remove-section ("submodule." + $SubmodulePath) 2>$null
}

# Step 4: Remove section from .git/config
Write-Host "→ Removing entry from .git/config..."
git config -f .git/config --remove-section ("submodule." + $SubmodulePath) 2>$null

# Step 5: Delete .git/modules/<submodule>
$moduleDir = ".git/modules/$SubmodulePath"
if (Test-Path $moduleDir) {
    Write-Host "→ Removing $moduleDir..."
    Remove-Item -Recurse -Force $moduleDir
}

# Step 6: Commit the cleanup
Write-Hos
