let
  pkgs = import (fetchTarball {
    url =
      "https://github.com/NixOS/nixpkgs/archive/63dacb46bf939521bdc93981b4cbb7ecb58427a0.tar.gz";
    sha256 = "sha256:1lr1h35prqkd1mkmzriwlpvxcb34kmhc9dnr48gkm8hh089hifmx";
  }) { };
in {
  inherit pkgs;
  stdenv = pkgs.stdenv;
  mkShell = pkgs.mkShell;

  inputs = with pkgs; [ cmake nlohmann_json tl-expected fmt cxxopts ];
}
