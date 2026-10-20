#include "cluster/cluster_data.hpp"
#include <cstdio>
#include <filesystem>
#include <netcdf>
#include <optional>
#include <print>

namespace fs = std::filesystem;

std::optional<ClusterInput> readClusterInput(const fs::path &input_path) {
  try {
    netCDF::NcFile file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcDim pixel_dim = file.getDim("pixel");
    netCDF::NcDim time_dim = file.getDim("time");
    if (pixel_dim.isNull() || time_dim.isNull()) {
      std::println(stderr, "Missing 'time' or 'pixel' dimension in {}.",
                   input_path.string());
      return std::nullopt;
    }

    ClusterInput data;
    data.num_rows = pixel_dim.getSize();
    data.num_cols = time_dim.getSize();

    netCDF::NcVar lat_var = file.getVar("latitude");
    netCDF::NcVar lon_var = file.getVar("longitude");
    netCDF::NcVar time_var = file.getVar("time");
    netCDF::NcVar sst_var = file.getVar("sst");

    if (lat_var.isNull() || lon_var.isNull() || time_var.isNull() ||
        sst_var.isNull()) {
      std::println(stderr, "Missing expected variables in {}.",
                   input_path.string());
      return std::nullopt;
    }

    data.lats.resize(data.num_rows);
    data.lons.resize(data.num_rows);
    data.days.resize(data.num_cols);
    data.sst.resize(data.num_rows * data.num_cols);

    lat_var.getVar(data.lats.data());
    lon_var.getVar(data.lons.data());

    std::vector<int> raw_days(data.num_cols);
    time_var.getVar(raw_days.data());
    std::copy(raw_days.begin(), raw_days.end(), data.days.data());

    sst_var.getVar(data.sst.data());

    return data;
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "NetCDF Error [{}]: {}", input_path.string(),
                 exception.what());
    return std::nullopt;
  }
}
