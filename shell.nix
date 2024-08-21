let
  pkgs = import ./packages.nix;
in
with pkgs;

mkShell
  {
    stdenv = pkgs.stdenv;
    nativeInputs = pkgs.inputs;
  }
