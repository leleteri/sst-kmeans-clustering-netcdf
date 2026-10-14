#include "cluster/cluster_data.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <netcdf>
#include <optional>

namespace fs = std::filesystem;

std::optional<ClusterInput> readClusterInput(const fs::path &path) {
  try {
    netCDF::NcFile file(path.string(), netCDF::NcFile::read);
    netCDF::NcDim pixel_dim = file.getDim("pixel");
    netCDF::NcDim time_dim = file.getDim("time");
    if (pixel_dim.isNull() || time_dim.isNull()) {
      std::cerr << "Missing 'time' or 'pixel' dimension in " << path << "\n";
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
      std::cerr << "Missing expected variables in " << path << "\n";
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
  } catch (const netCDF::exceptions::NcException &exceptions) {
    std::cerr << "NetCDF Error [" << path << "]" << exceptions.what() << "\n";
    return std::nullopt;
  }
}
