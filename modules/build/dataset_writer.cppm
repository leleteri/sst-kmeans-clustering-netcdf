module;
#include <netcdf>

export module build.dataset_writer;

import std;
import sst_schema;
import nc_utils;
import build.dataset;
import build.grid_info;
import build.time_axis;

namespace fs = std::filesystem;

// Writes a Dataset to a NetCDF-4 file.
//   analysed_sst(pixel, time)   latitude(pixel)   longitude(pixel)   time(time)
export class DatasetWriter {
public:
  explicit DatasetWriter(fs::path output, bool verbose = false)
      : m_output(std::move(output)), m_verbose(verbose) {}

  void write(const Dataset &dataset, const GridInfo &grid,
             const TimeAxis &time_axis) const {
    const std::size_t num_pixels = grid.numOceanCells();
    const std::size_t num_days = time_axis.dayOffsets().size();

    if (num_pixels == 0 || num_days == 0 || dataset.numPixels() != num_pixels ||
        dataset.numDays() != num_days ||
        dataset.matrix().size() != num_pixels * num_days) {
      throw std::runtime_error(
          "cannot write dataset: matrix size does not match pixels x days.");
    }

    const auto [pixel_lat, pixel_lon] = pixelCoordinates(grid, num_pixels);

    ncGuard(m_output.string(), [&] {
      netCDF::NcFile out(m_output.string(), netCDF::NcFile::replace,
                         netCDF::NcFile::nc4);

      const netCDF::NcDim pixel_dim =
          out.addDim(std::string(schema::PixelDim), num_pixels);
      const netCDF::NcDim time_dim =
          out.addDim(std::string(schema::TimeDim), num_days);

      const netCDF::NcVar lat_var = out.addVar(std::string(schema::LatitudeVar),
                                               netCDF::ncDouble, pixel_dim);
      const netCDF::NcVar lon_var = out.addVar(
          std::string(schema::LongitudeVar), netCDF::ncDouble, pixel_dim);
      const netCDF::NcVar time_var =
          out.addVar(std::string(schema::TimeVar), netCDF::ncInt, time_dim);
      const netCDF::NcVar sst_var =
          out.addVar(std::string(schema::SstVar), netCDF::ncFloat,
                     std::vector<netCDF::NcDim>{pixel_dim, time_dim});

      lat_var.putAtt("units", "degrees_north");
      lat_var.putAtt("standard_name", "latitude");
      lat_var.putAtt("long_name", "latitude of the ocean pixel");
      lat_var.putAtt("axis", "Y");

      lon_var.putAtt("units", "degrees_east");
      lon_var.putAtt("standard_name", "longitude");
      lon_var.putAtt("long_name", "longitude of the ocean pixel");
      lon_var.putAtt("axis", "X");

      time_var.putAtt("units", std::string(schema::TimeUnits));
      time_var.putAtt("calendar", std::string(schema::TimeCalendar));
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

      // The fill value must be passed by value: a pointer selects the
      // setFill(bool, void*) overload, which does not store the value.
      sst_var.setFill(true, std::numeric_limits<float>::quiet_NaN());
      std::vector<std::size_t> chunks = {std::min<std::size_t>(num_pixels, 128),
                                         std::min<std::size_t>(num_days, 64)};
      sst_var.setChunking(netCDF::NcVar::nc_CHUNKED, chunks);
      sst_var.setCompression(true, true, 1); // shuffle + deflate level 1

      lat_var.putVar(pixel_lat.data());
      lon_var.putVar(pixel_lon.data());
      time_var.putVar(time_axis.dayOffsets().data());
      sst_var.putVar(dataset.matrix().data()); // row-major [pixel][time]
    });

    if (m_verbose) {
      std::println("wrote {} x {} dataset to {}.", num_pixels, num_days,
                   m_output.string());
    }
  }

private:
  fs::path m_output;
  bool m_verbose;

  // Latitude and longitude of every ocean pixel, in dataset row order
  // (row-major scan of the lat x lon grid, ocean cells only).
  static std::pair<std::vector<double>, std::vector<double>>
  pixelCoordinates(const GridInfo &grid, std::size_t num_pixels) {
    const std::vector<bool> &mask = grid.oceanMask();
    const std::size_t num_lons = grid.longitude().size();

    std::vector<double> lats;
    std::vector<double> lons;
    lats.reserve(num_pixels);
    lons.reserve(num_pixels);
    for (std::size_t cell = 0; cell < mask.size(); ++cell) {
      if (!mask[cell])
        continue;
      lats.push_back(grid.latitude()[cell / num_lons]);
      lons.push_back(grid.longitude()[cell % num_lons]);
    }
    return {std::move(lats), std::move(lons)};
  }
};
