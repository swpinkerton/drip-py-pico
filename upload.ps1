param (
    [string]$filename = "main.uf2",
    [string]$pipath = "D:\"
)

$bin_path = ".\build\src\"

$source_path = Join-Path -Path $bin_path -ChildPath $filename

try {
    Copy-Item -Path $source_path -Destination $pipath -ErrorAction Stop
    Write-Output "File uploaded to the pi."
} catch {
    Write-Output "Failed to upload the file. Path $pipath not found."
}