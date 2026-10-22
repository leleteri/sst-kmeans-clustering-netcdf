#include "build/build_output.hpp"
#include "build/sst_data.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <netcdf>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const char *kTimeUnits = "days since 1970-01-01";
constexpr const char *kTimeCalendar = "proleptic_gregorian";

} // namespace

bool writeDatasetNc(const SstDataset &dataset, const fs::path &output_path,
                    bool verbose) {
  const std::vector<bool> &mask = dataset.grid.oceanMask;
  const size_t num_lons = dataset.grid.coords.lons.size();
  const size_t num_days = dataset.time.dayOffsets.size();
  const size_t num_rows = std::count(mask.begin(), mask.end(), true);

  if (num_rows == 0 || num_days == 0 || num_lons == 0 ||
      dataset.matrix.size() != num_rows * num_days) {
    std::println(stderr,
                 "Error: Cannot write dataset: matrix size does not match "
                 "rows x days.");
    return false;
  }

  std::vector<double> row_lat;
  std::vector<double> row_lon;
  row_lat.reserve(num_rows);
  row_lon.reserve(num_rows);
  for (size_t g = 0; g < mask.size(); ++g) {
    if (!mask[g])
      continue;
    row_lat.push_back(dataset.grid.coords.lats[g / num_lons]);
    row_lon.push_back(dataset.grid.coords.lons[g % num_lons]);
  }

  std::vector<int> days;
  days.reserve(num_days);
  for (long offset : dataset.time.dayOffsets) {
    if (offset < static_cast<long>(std::numeric_limits<int>::min()) ||
        offset > static_cast<long>(std::numeric_limits<int>::max())) {
      std::println(stderr,
                   "Error: Day offset {} does not fit in the 32-bit time "
                   "variable.",
                   offset);
      return false;
    }
    days.push_back(static_cast<int>(offset));
  }

  try {
    netCDF::NcFile out(output_path.string(), netCDF::NcFile::replace,
                       netCDF::NcFile::nc4);

    netCDF::NcDim pixel_dim = out.addDim("pixel", num_rows);
    netCDF::NcDim time_dim = out.addDim("time", num_days);

    netCDF::NcVar lat_var = out.addVar("latitude", netCDF::ncDouble, pixel_dim);
    netCDF::NcVar lon_var =
        out.addVar("longitude", netCDF::ncDouble, pixel_dim);
    netCDF::NcVar time_var = out.addVar("time", netCDF::ncInt, time_dim);

    std::vector<netCDF::NcDim> sst_dims = {pixel_dim, time_dim};
    netCDF::NcVar sst_var =
        out.addVar("analysed_sst", netCDF::ncFloat, sst_dims);

    lat_var.putAtt("units", "degrees_north");
    lat_var.putAtt("standard_name", "latitude");
    lat_var.putAtt("long_name", "latitude of the ocean pixel");
    lat_var.putAtt("axis", "Y");
    lon_var.putAtt("units", "degrees_east");
    lon_var.putAtt("standard_name", "longitude");
    lon_var.putAtt("long_name", "longitude of the ocean pixel");
    lon_var.putAtt("axis", "X");
    time_var.putAtt("units", kTimeUnits);
    time_var.putAtt("calendar", kTimeCalendar);
    time_var.putAtt("standard_name", "time");
    time_var.putAtt("long_name", "days since 1970-01-01");
    time_var.putAtt("axis", "T");
    sst_var.putAtt("units", "degree_Celsius");
    sst_var.putAtt("long_name", "analysed sea surface temperature");
    sst_var.putAtt("standard_name", "sea_surface_temperature");
    out.putAtt("title", "Daily SST, one row per ocean pixel, one column per "
                        "file/day");

    if (!dataset.grid.crs_wkt.empty()) {
      netCDF::NcVar crs_var = out.addVar("crs", netCDF::ncInt);
      crs_var.putAtt("crs_wkt", dataset.grid.crs_wkt);
      sst_var.putAtt("grid_mapping", "crs");
    }

    // NOTE: the fill value must be passed by value. Passing a pointer selects
    // the template overload setFill(bool, T), which forwards the address of
    // the pointer instead of the value and stores a garbage fill value.
    const float nan_fill = std::numeric_limits<float>::quiet_NaN();
    sst_var.setFill(true, nan_fill);

    // Chunk by time so a chunk stays well below the HDF5 chunk cache size.
    const size_t chunk_rows = std::min<size_t>(num_rows, 128);
    const size_t chunk_days = std::min<size_t>(num_days, 64);
    std::vector<size_t> chunks = {chunk_rows, chunk_days};
    sst_var.setChunking(netCDF::NcVar::nc_CHUNKED, chunks);
    sst_var.setCompression(true, true, 1); // shuffle + deflate level 1

    lat_var.putVar(row_lat.data());
    lon_var.putVar(row_lon.data());
    time_var.putVar(days.data());
    sst_var.putVar(dataset.matrix.data());

  } catch (const netCDF::exceptions::NcException &e) {
    std::println(stderr, "Error: {}: NetCDF: {}.", output_path.string(),
                 e.what());
    return false;
  } catch (const std::exception &e) {
    std::println(stderr, "Error: {}: {}.", output_path.string(), e.what());
    return false;
  }

  if (verbose)
    std::println("Wrote {} x {} dataset to {}.", num_rows, num_days,
                 output_path.string());
  return true;
}
