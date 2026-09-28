#include "dataset_io.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <netcdf>
#include <vector>

bool writeDatasetNc(const SstDataset &dataset, const fs::path &output_path,
                    bool verbose) {
  const std::vector<bool> &mask = dataset.grid.oceanMask;
  const size_t num_lons = dataset.grid.coords.lons.size();
  const size_t num_days = dataset.time.dayOffsets.size();
  const size_t num_rows = std::count(mask.begin(), mask.end(), true);

  if (num_rows == 0 || num_days == 0 ||
      dataset.matrix.size() != num_rows * num_days) {
    if (verbose)
      std::cerr << "Cannot write dataset: matrix size does not match "
                   "rows x days.\n";
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
  for (long offset : dataset.time.dayOffsets)
    days.push_back(static_cast<int>(offset));

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
    netCDF::NcVar sst_var = out.addVar("sst", netCDF::ncFloat, sst_dims);

    lat_var.putAtt("units", "degrees_north");
    lon_var.putAtt("units", "degrees_east");
    time_var.putAtt("units", "days since 1970-01-01");
    time_var.putAtt("calendar", "standard");
    sst_var.putAtt("units", "degree_Celsius");
    sst_var.putAtt("long_name", "analysed sea surface temperature");
    out.putAtt("title", "Daily SST, one row per ocean pixel, one column per "
                        "file/day");

    const float nan_fill = std::numeric_limits<float>::quiet_NaN();
    sst_var.setFill(true, &nan_fill);

    const size_t chunk_rows = std::min<size_t>(num_rows, 128);
    std::vector<size_t> chunks = {chunk_rows, num_days};
    sst_var.setChunking(netCDF::NcVar::nc_CHUNKED, chunks);
    sst_var.setCompression(true, true, 1); // shuffle + deflate level 1

    lat_var.putVar(row_lat.data());
    lon_var.putVar(row_lon.data());
    time_var.putVar(days.data());
    sst_var.putVar(dataset.matrix.data());

  } catch (const netCDF::exceptions::NcException &e) {
    if (verbose)
      std::cerr << "NetCDF write error [" << output_path << "]: " << e.what()
                << "\n";
    return false;
  } catch (const std::exception &e) {
    if (verbose)
      std::cerr << "Write error [" << output_path << "]: " << e.what() << "\n";
    return false;
  }

  if (verbose)
    std::cout << "Wrote " << num_rows << " x " << num_days << " dataset to "
              << output_path << "\n";
  return true;
}
