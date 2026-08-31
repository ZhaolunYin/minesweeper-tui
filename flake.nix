{
    description = "A basic dev shell";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    };

    outputs = { nixpkgs, self, ... }:
    let
        forAllSystems = function:
        nixpkgs.lib.genAttrs [
            "x86_64-linux"
            "aarch64-linux"
            "x86_64-darwin"
            "aarch64-darwin"
        ] (system: function nixpkgs.legacyPackages.${system});
    in
    {
        packages = forAllSystems (pkgs: {
            default = pkgs.stdenv.mkDerivation {
                pname = "minesweeper-tui";
                version = "0.4.0";
                src = ./.;

                nativeBuildInputs = [ pkgs.pkg-config ];
                buildInputs = [ pkgs.ncurses ];

                strictDeps = true;
                __structuredAttrs = true;

                installPhase = ''
                    install -Dm755 ./build/minesweeper-tui $out/bin/minesweeper-tui
                '';
            };
        });
        homeModules.default = { pkgs, ... }: {
            home.packages = [
                self.packages.${pkgs.system}.default
            ];
        };
        devShells.x86_64-linux.default = 
            nixpkgs.legacyPackages.x86_64-linux.mkShell {
                packages = with nixpkgs.legacyPackages.x86_64-linux; [
                    bear
                    gcc
                    ncurses
                    pkg-config
                ];
                shellHook = ''
                    exec zsh
                '';
            };
    };
}
