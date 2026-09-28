#include "grid_data.hpp"
#include <filesystem>
#include <iostream>
#include <netcdf>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

std::optional<GridInfo> readGridInfo(const fs::path &input_path, bool verbose) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar lat_var = data_file.getVar("latitude");
    netCDF::NcVar lon_var = data_file.getVar("longitude");
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");

    if (lat_var.isNull() || lon_var.isNull() || sst_var.isNull()) {
      if (verbose)
        std::cerr
            << "Error [" << input_path.filename()
            << "]: Required variables (latitude, longitude, analysed_sst) "
               "not found.\n";
      return std::nullopt;
    }

    size_t num_lats = lat_var.getDim(0).getSize();
    size_t num_lons = lon_var.getDim(0).getSize();

    GridInfo grid;
    grid.coords.lats.resize(num_lats);
    grid.coords.lons.resize(num_lons);
    lat_var.getVar(grid.coords.lats.data());
    lon_var.getVar(grid.coords.lons.data());

    netCDF::NcVarAtt fillAtt = sst_var.getAtt("_FillValue");
    fillAtt.getValues(&grid.fill_value);

    std::vector<float> sst(num_lats * num_lons);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, num_lats, num_lons};
    sst_var.getVar(start, count, sst.data());

    grid.oceanMask.resize(num_lats * num_lons);
    for (size_t k = 0; k < sst.size(); ++k) {
      grid.oceanMask[k] = (sst[k] != grid.fill_value);
    }

    return grid;
  } catch (const netCDF::exceptions::NcException &exception) {
    if (verbose)
      std::cerr << "NetCDF Error [" << input_path.filename() << "]"
                << exception.what();
    return std::nullopt;
  }
}

std::optional<std::vector<float>>
readSstValues(const fs::path &input_path, size_t expectedSize, bool verbose) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");

    if (sst_var.isNull()) {
      if (verbose)
        std::cerr << "Error [" << input_path.filename()
                  << "]: analysed_sst not found.\n";
      return std::nullopt;
    }

    size_t num_lats = data_file.getDim("latitude").getSize();
    size_t num_lons = data_file.getDim("longitude").getSize();
    if (num_lats * num_lons != expectedSize) {
      if (verbose)
        std::cerr << "Grid size mismatch in " << input_path.filename() << "\n";
      return std::nullopt;
    }

    std::vector<float> sst(expectedSize);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, num_lats, num_lons};

    sst_var.getVar(start, count, sst.data());
    return sst;
  } catch (const netCDF::exceptions::NcException &e) {
    if (verbose)
      std::cerr << "NetCDF Error [" << input_path.filename()
                << "]: " << e.what() << "\n";
    return std::nullopt;
  }
}

std::optional<LatLon> readLatLon(const fs::path &input_path, bool verbose) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar lat_var = data_file.getVar("latitude");
    netCDF::NcVar lon_var = data_file.getVar("longitude");

    if (lat_var.isNull() || lon_var.isNull()) {
      if (verbose)
        std::cerr << "Error [" << input_path.filename()
                  << "]: Required variables (latitude, longitude) "
                     "not found.\n";
      return std::nullopt;
    }

    LatLon result;
    result.lats.resize(lat_var.getDim(0).getSize());
    result.lons.resize(lon_var.getDim(0).getSize());
    lat_var.getVar(result.lats.data());
    lon_var.getVar(result.lons.data());
    return result;
  } catch (const netCDF::exceptions::NcException &exception) {
    if (verbose)
      std::cerr << "NetCDF Error [" << input_path.filename() << "]"
                << exception.what();
    return std::nullopt;
  }
}

std::optional<std::vector<bool>> readOceanMask(const fs::path &input_path,
                                               bool verbose) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");
    if (sst_var.isNull()) {
      if (verbose)
        std::cerr << "Error [" << input_path.filename()
                  << "]: analysed_sst not found.\n";
      return std::nullopt;
    }

    netCDF::NcDim lat_dim = data_file.getDim("latitude");
    netCDF::NcDim lon_dim = data_file.getDim("longitude");
    size_t num_lats = lat_dim.getSize();
    size_t num_lons = lon_dim.getSize();

    std::vector<float> sst(num_lats * num_lons);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, num_lats, num_lons};
    sst_var.getVar(start, count, sst.data());

    float fill_value;
    netCDF::NcVarAtt fillAtt = sst_var.getAtt("_FillValue");
    fillAtt.getValues(&fill_value);

    std::vector<bool> mask(sst.size());
    for (size_t i = 0; i < sst.size(); ++i)
      mask[i] = (sst[i] != fill_value);
    return mask;

  } catch (const netCDF::exceptions::NcException &e) {
    if (verbose)
      std::cerr << "NetCDF Error [" << input_path.filename()
                << "]: " << e.what() << "\n";
    return std::nullopt;
  }
}
