{
  pkgs,
  lib,
  ...
}: let
  llvm = pkgs.llvmPackages_22;
  netcdfcxx4-libcxx = pkgs.netcdfcxx4.override {
    stdenv = llvm.libcxxStdenv;
  };
in {
  stdenv = llvm.libcxxStdenv;

  packages = [
    pkgs.git
    pkgs.pkg-config
    pkgs.ninja
    pkgs.cmake
    pkgs.netcdf
    netcdfcxx4-libcxx
    llvm.libcxx
    llvm.clang-tools
  ];

  env.LIBCXX_MODULES_JSON = "${lib.getLib llvm.libcxx}/lib/libc++.modules.json";

  env.CXXFLAGS = lib.concatStringsSep " " [
    "-isystem ${lib.getDev llvm.libcxx}/include/c++/v1"
    "-isystem ${llvm.libcxxStdenv.cc.libc_dev}/include"
  ];

  enterShell = ''
    export NIX_HARDENING_ENABLE="$(echo "$NIX_HARDENING_ENABLE" | tr ' ' '\n' | grep -vxE 'fortify3?' | tr '\n' ' ')"
  '';
}
