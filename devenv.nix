{
  pkgs,
  lib,
  config,
  inputs,
  ...
}: {
  packages = [
    pkgs.git
    pkgs.pkg-config
    pkgs.clang-tools
    pkgs.gcc
    pkgs.cmake
    pkgs.netcdf
    pkgs.netcdfcxx4
  ];

  languages.c.enable = true;
  languages.cplusplus.enable = true;

  env.CXXFLAGS = "-std=c++20";
}
