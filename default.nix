let pkgs = import ./packages.nix;
in pkgs.stdenv.mkDerivation {
  pname = "wolfpack";
  version = "0.0.1";
  src = ./.;

  nativeBuildInputs = pkgs.inputs;

  configurePhase = ''
    cmake . \
    -DCMAKE_INSTALL_PREFIX=$out \
    -DCMAKE_BUILD_TYPE=Release \
    -DDONT_FETCH_PKGS=ON
  '';

  buildPhase = ''
    cmake --build . --parallel
  '';

  installPhase = ''
    cmake --build . --parallel --target install
  '';
}
