$sourceName = "nuget.config.officialbuild"
$destinationName = "nuget.config"

# Copy over the destination file
Copy-Item -Path $sourceName -Destination $destinationName -Force