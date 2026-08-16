{
    description = "A basic dev shell";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    };

    outputs = { nixpkgs, ... }:
    {
        devShells.x86_64-linux.default = 
            nixpkgs.legacyPackages.x86_64-linux.mkShell {
                packages = with nixpkgs.legacyPackages.x86_64-linux; [
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
