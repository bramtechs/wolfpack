./install.ps1

Push-Location example
../bin/wolfpack --verbose
cmake --preset example
cmake --build build --parallel
Pop-Location