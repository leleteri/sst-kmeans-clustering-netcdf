module;
#include <netcdf>

export module build_output;

import std;
import dataset;
import grid_info;
import time_axis;

namespace fs = std::filesystem;

namespace {

constexpr const char *kTimeUnits = "days since 1970-01-01";
constexpr const char *kTimeCalendar = "proleptic_gregorian";

} // namespace

// Writes the (ocean pixel x day) matrix to a NetCDF-4 file.
// Layout: analysed_sst(pixel, time); latitude/longitude(pixel); time(time).
export bool writeDatasetNc(const Dataset &dataset, const GridInfo &grid,
                           const TimeAxis &time, const fs::path &output_path,
                           bool verbose = false) {
  const std::vector<bool> &mask = grid.oceanMask();
  const std::vector<double> &lats = grid.latitude();
  const std::vector<double> &lons = grid.longitude();
  const std::vector<long> &offsets = time.dayOffsets();

  const std::size_t num_lats = lats.size();
  const std::size_t num_lons = lons.size();
  const std::size_t num_days = offsets.size();
  const std::size_t num_rows =
      static_cast<std::size_t>(std::count(mask.begin(), mask.end(), true));

  if (num_rows == 0 || num_days == 0 || num_lons == 0 || num_lats == 0 ||
      mask.size() != num_lats * num_lons || dataset.numRows() != num_rows ||
      dataset.numDays() != num_days ||
      dataset.matrix().size() != num_rows * num_days) {
    std::println(stderr, "Error: Cannot write dataset: matrix size does not "
                         "match rows x days.");
    return false;
  }

  // Per-row coordinates, in the same order the dataset rows were built
  // (row-major scan over the lat x lon grid, ocean pixels only).
  std::vector<double> row_lat;
  std::vector<double> row_lon;
  row_lat.reserve(num_rows);
  row_lon.reserve(num_rows);
  for (std::size_t g = 0; g < mask.size(); ++g) {
    if (!mask[g])
      continue;
    row_lat.push_back(lats[g / num_lons]);
    row_lon.push_back(lons[g % num_lons]);
  }

  std::vector<int> days;
  days.reserve(num_days);
  for (long offset : offsets) {
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

    const std::vector<netCDF::NcDim> sst_dims = {pixel_dim, time_dim};
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
    time_var.putAtt("long_name", "time");
    time_var.putAtt("axis", "T");

    sst_var.putAtt("units", "degree_Celsius");
    sst_var.putAtt("long_name", "analysed sea surface temperature");
    sst_var.putAtt("standard_name", "sea_surface_temperature");
    sst_var.putAtt("coordinates", "latitude longitude");

    out.putAtt("title",
               "Daily SST, one row per ocean pixel, one column per day");
    out.putAtt("Conventions", "CF-1.8");

    // NOTE: pass the fill value by value, not by pointer (see setFill(bool,
    // T)).
    const float nan_fill = std::numeric_limits<float>::quiet_NaN();
    sst_var.setFill(true, nan_fill);

    std::vector<std::size_t> chunks = {std::min<std::size_t>(num_rows, 128),
                                       std::min<std::size_t>(num_days, 64)};
    sst_var.setChunking(netCDF::NcVar::nc_CHUNKED, chunks);
    sst_var.setCompression(true, true, 1); // shuffle + deflate level 1

    lat_var.putVar(row_lat.data());
    lon_var.putVar(row_lon.data());
    time_var.putVar(days.data());
    sst_var.putVar(dataset.matrix().data()); // row-major [pixel][time]

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
