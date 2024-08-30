let
  pkgs = import ./packages.nix;
  wolfpack = import ./.;
in
with pkgs.pkgs;
pkgs.mkShell {
  stdenv = pkgs.stdenv;
  nativeBuildInputs = with pkgs; [ git ] ++ pkgs.inputs;
  buildInputs = [ wolfpack ];
}
