{
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    forEachSystem = fn:
      nixpkgs.lib.genAttrs
      ["x86_64-linux" "aarch64-linux"]
      (system: fn system nixpkgs.legacyPackages.${system});
  in {
    packages = forEachSystem (system: pkgs: rec {
      quickshell = pkgs.callPackage ./default.nix {
        gitRev = self.rev or self.dirtyRev;
      };
      quickshell-nvidia = quickshell.override {nvidiaCompat = true;};

      default = quickshell;
      nvidia = quickshell-nvidia;
    });

    devShells = forEachSystem (system: pkgs: rec {
      default = import ./shell.nix {
        inherit pkgs;
        inherit (self.packages.${system}) quickshell;
      };
    });

    homeManagerModules.default = import ./hm-module.nix self;
  };
}
