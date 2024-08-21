let pkgs = import ./packages.nix;
in with pkgs;
pkgs.mkShell {
  stdenv = pkgs.stdenv;
  buildInputs = [ pkgs.git ] ++ pkgs.inputs;
}
