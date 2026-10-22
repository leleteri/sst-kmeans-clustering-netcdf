#include "cluster/cluster_data.hpp"
#include "map_data.hpp"

#include <cstdio>
#include <filesystem>
#include <netcdf>
#include <optional>
#include <print>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

std::optional<ClusterInput> readClusterInput(const fs::path &input_path) {
  const std::string file_name = input_path.string();
  try {
    netCDF::NcFile file(file_name, netCDF::NcFile::read);
    netCDF::NcDim pixel_dim = file.getDim("pixel");
    netCDF::NcDim time_dim = file.getDim("time");
    if (pixel_dim.isNull() || time_dim.isNull()) {
      std::println(stderr, "Error: Missing 'time' or 'pixel' dimension in {}.",
                   file_name);
      return std::nullopt;
    }

    netCDF::NcVar time_var = file.getVar("time");
    if (time_var.isNull()) {
      std::println(stderr, "Error: Missing expected variables in {}.",
                   file_name);
      return std::nullopt;
    }

    ClusterInput data;
    data.num_rows = pixel_dim.getSize();
    data.num_cols = time_dim.getSize();

    auto coords = readLatLon(file, file_name);
    if (!coords)
      return std::nullopt;
    if (coords->lats.size() != data.num_rows ||
        coords->lons.size() != data.num_rows) {
      std::println(
          stderr,
          "Error: Latitude/longitude size does not match 'pixel' in {}.",
          file_name);
      return std::nullopt;
    }
    data.coords = std::move(*coords);

    std::vector<int> raw_days(data.num_cols);
    time_var.getVar(raw_days.data());
    data.days.assign(raw_days.begin(), raw_days.end());

    auto sst = readSstValues(file, file_name, {data.num_rows, data.num_cols});
    if (!sst)
      return std::nullopt;
    data.sst = std::move(*sst);

    return data;
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: NetCDF Error [{}]: {}", file_name,
                 exception.what());
    return std::nullopt;
  }
}
