{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        shellApplication = name:
          pkgs.writeShellApplication {
            name = "${name}";
            text =
              let
                binPath = pkgs.lib.makeBinPath [ ];
              in
              ''
                #!${pkgs.runtimeShell}
                export PATH="${binPath}:$PATH"
              '' + builtins.readFile (./. + "/${name}");
            runtimeInputs = [
              pkgs.curl
              pkgs.jq
              pkgs.openssl
              pkgs.mpv
              pkgs.dialog
              pkgs.python311Packages.eyeD3
              pkgs.pv
            ];
            checkPhase = "${pkgs.stdenv.shellDryRun} $target";
            bashOptions = [ ];
          };
      in
      {
        packages.default = (shellApplication "dzr");
        packages.dzr-dec = (shellApplication "dzr-dec");
        packages.dzr-id3 = (shellApplication "dzr-id3");
        packages.dzr-srt = (shellApplication "dzr-srt");
        packages.dzr-url = (shellApplication "dzr-url");
      }
    );
}
