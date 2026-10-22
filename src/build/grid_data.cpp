#include "build/grid_data.hpp"
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

namespace {

std::optional<std::string> readCrsWkt(netCDF::NcFile &file) {
  netCDF::NcVar crs_var = file.getVar("crs");
  if (crs_var.isNull())
    return std::nullopt;

  netCDF::NcVarAtt wkt_att = crs_var.getAtt("crs_wkt");
  if (wkt_att.isNull())
    return std::nullopt;

  std::string wkt;
  wkt_att.getValues(wkt);
  return wkt;
}

} // namespace

std::optional<float> readFillValue(netCDF::NcFile &file,
                                    const std::string &file_name) {
  netCDF::NcVar sst_var = file.getVar("analysed_sst");
  if (sst_var.isNull()) {
    std::println(stderr, "Error: {}: Required variable analysed_sst not found.",
                 file_name);
    return std::nullopt;
  }

  netCDF::NcVarAtt fill_att = sst_var.getAtt("_FillValue");
  if (fill_att.isNull()) {
    std::println(stderr, "Error: {}: analysed_sst has no _FillValue.",
                 file_name);
    return std::nullopt;
  }

  float fill_value = 0.0f;
  fill_att.getValues(&fill_value);
  return fill_value;
}

std::optional<float>
readFillValue(const std::filesystem::path &input_path) {
  const std::string file_name = input_path.filename().string();
  try {
    netCDF::NcFile file(input_path.string(), netCDF::NcFile::read);
    return readFillValue(file, file_name);
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: {}: NetCDF: {}.", file_name, exception.what());
    return std::nullopt;
  }
}

std::optional<GridInfo> readGridInfo(const fs::path &input_path) {
  const std::string file_name = input_path.filename().string();
  try {
    netCDF::NcFile file(input_path.string(), netCDF::NcFile::read);

    auto coords = readLatLon(file, file_name);
    if (!coords)
      return std::nullopt;

    auto sst = readSstFirstStep(file, file_name,
                                {coords->lats.size(), coords->lons.size()});
    if (!sst)
      return std::nullopt;

    auto fill_value = readFillValue(file, file_name);
    if (!fill_value)
      return std::nullopt;

    GridInfo grid;
    grid.coords = std::move(*coords);
    grid.fill_value = *fill_value;
    grid.oceanMask.resize(sst->size());
    for (size_t i = 0; i < sst->size(); ++i) {
      grid.oceanMask[i] = ((*sst)[i] != grid.fill_value);
    }
    if (auto wkt = readCrsWkt(file))
      grid.crs_wkt = std::move(*wkt);
    return grid;
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: {}: NetCDF: {}.", file_name, exception.what());
    return std::nullopt;
  }
}
