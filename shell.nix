let pkgs = import ./packages.nix;
in pkgs.mkShell {
  stdenv = pkgs.stdenv;
  nativeInputs = pkgs.inputs;
}
