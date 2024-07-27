{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell
  {
    stdenv = pkgs.llvmPackages_18.libcxxStdenv;

    buildInputs = [
      llvmPackages_18.clangUseLLVM
      lldb
    ];
  }
