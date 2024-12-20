{
  description = "Experimental implementation of Deque programming language.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
    llvmPkgs = pkgs.llvmPackages;
  in {
    devShells.${system}.default =
      pkgs.mkShell.override {
        stdenv = llvmPkgs.libcxxStdenv;
      }
      {
        packages = with pkgs; [
          gnumake
          mold
          llvmPkgs.libcxx
          llvmPkgs.clang-tools
          llvmPkgs.libcxxClang
          llvmPkgs.bintools
        ];
      };
  };
}
