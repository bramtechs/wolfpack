./install.ps1

Push-Location tests/01
../../bin/wolfpack --verbose
cmake --preset example
cmake --build build --parallel
Pop-Location

Push-Location tests/02
../../bin/wolfpack --verbose
cmake --preset example
cmake --build build --parallel
Pop-Location