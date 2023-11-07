{
  pkgs ? import (fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/4fe8d07066f6ea82cda2b0c9ae7aee59b2d241b3.tar.gz";
    sha256 = "sha256:06jzngg5jm1f81sc4xfskvvgjy5bblz51xpl788mnps1wrkykfhp";
  }) {}
}: pkgs.stdenv.mkDerivation rec {
  pname = "fastfetch";
  version = "2.2.2";

  src = pkgs.fetchgit {
    url = "https://github.com/fastfetch-cli/fastfetch";
    rev = "864e707224cc784c637cf8f999bc176bc8f0f65e";
    sha256 = "sha256-LItQJRpaelOT6Kdof8SyWBsbwI80cqKNmGkvKZszqd4=";
  };

  buildInputs = with pkgs; [
    pkg-config
    cmake
    clang
    vulkan-loader
    opencl-headers
    ocl-icd
  ];
}

