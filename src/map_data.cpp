#include "map_data.hpp"

#include <cstdio>
#include <filesystem>
#include <netcdf>
#include <optional>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::optional<LatLon> readLatLon(netCDF::NcFile &file,
                                 const std::string &file_name) {
  netCDF::NcVar lat_var = file.getVar("latitude");
  netCDF::NcVar lon_var = file.getVar("longitude");

  if (lat_var.isNull() || lon_var.isNull()) {
    std::println(
        stderr,
        "Error: {}: Required variables (latitude, longitude) not found.",
        file_name);
    return std::nullopt;
  }

  LatLon result;
  result.lats.resize(lat_var.getDim(0).getSize());
  result.lons.resize(lon_var.getDim(0).getSize());
  lat_var.getVar(result.lats.data());
  lon_var.getVar(result.lons.data());
  return result;
}

std::optional<std::vector<float>>
readSstValues(netCDF::NcFile &file, const std::string &file_name,
              const std::vector<size_t> &shape) {
  netCDF::NcVar sst_var = file.getVar("analysed_sst");

  if (sst_var.isNull()) {
    std::println(stderr, "Error: {}: Required variable analysed_sst not found.",
                 file_name);
    return std::nullopt;
  }

  const std::vector<netCDF::NcDim> dims = sst_var.getDims();
  bool shape_matches = dims.size() == shape.size();
  for (size_t i = 0; shape_matches && i < dims.size(); ++i) {
    shape_matches = dims[i].getSize() == shape[i];
  }
  if (!shape_matches) {
    std::println(stderr,
                 "Error: {}: analysed_sst does not have the expected shape.",
                 file_name);
    return std::nullopt;
  }

  size_t total = 1;
  for (const size_t extent : shape) {
    total *= extent;
  }

  std::vector<float> values(total);
  const std::vector<size_t> start(shape.size(), 0);
  sst_var.getVar(start, shape, values.data());
  return values;
}

std::optional<LatLon> readLatLon(const fs::path &input_path) {
  const std::string file_name = input_path.filename().string();
  try {
    netCDF::NcFile file(input_path.string(), netCDF::NcFile::read);
    return readLatLon(file, file_name);
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: {}: NetCDF: {}.", file_name, exception.what());
    return std::nullopt;
  }
}

std::optional<std::vector<float>>
readSstValues(const fs::path &input_path, const std::vector<size_t> &shape) {
  const std::string file_name = input_path.filename().string();
  try {
    netCDF::NcFile file(input_path.string(), netCDF::NcFile::read);
    return readSstValues(file, file_name, shape);
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: {}: NetCDF: {}.", file_name, exception.what());
    return std::nullopt;
  }
}
