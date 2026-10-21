{
  pkgs,
  lib,
  ...
}: {
  stdenv = pkgs.llvmPackages_22.stdenv;

  packages = [
    pkgs.git
    pkgs.pkg-config
    pkgs.cmake
    pkgs.netcdf
    pkgs.netcdfcxx4
    pkgs.llvmPackages_22.clang-tools
  ];
}
