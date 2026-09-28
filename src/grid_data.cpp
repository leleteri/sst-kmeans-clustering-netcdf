#include "grid_data.hpp"
#include "exceptions.hpp"
#include <netcdf>
#include <vector>

std::optional<GridInfo> readGridInfo(const fs::path &input_path) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar lat_var = data_file.getVar("latitude");
    netCDF::NcVar lon_var = data_file.getVar("longitude");
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");

    if (sst_var.isNull()) {
      std::cerr << "Error [" << input_path.filename()
                << "]: Required variables (analysed_sst) "
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
    std::cerr << "NetCDF Error [" << input_path.filename() << "]"
              << exception.what();
    return std::nullopt;
  }
}

std::optional<std::vector<float>> readSstValues(const fs::path &input_path,
                                                size_t expectedSize) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");
    if (sst_var.isNull()) {
      std::cerr << "Error [" << input_path.filename()
                << "]: analysed_sst not found.\n";
      return std::nullopt;
    }

    std::vector<float> sst(expectedSize);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, /* num_lats */ 0,
                                 /* num_lons */ 0}; // fill from grid dims
    sst_var.getVar(start, count, sst.data());
    return sst;
  } catch (const netCDF::exceptions::NcException &e) {
    std::cerr << "NetCDF Error [" << input_path.filename() << "]: " << e.what()
              << "\n";
    return std::nullopt;
  }
}

std::optional<LatLon> readLatLon(const fs::path &input_path) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar lat_var = data_file.getVar("latitude");
    netCDF::NcVar lon_var = data_file.getVar("longitude");

    if (lat_var.isNull() || lon_var.isNull()) {
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
    std::cerr << "NetCDF Error [" << input_path.filename() << "]"
              << exception.what();
    return std::nullopt;
  }
}

std::optional<std::vector<bool>> readOceanMask(const fs::path &input_path) {
  try {
    netCDF::NcFile data_file(input_path.string(), netCDF::NcFile::read);
    netCDF::NcVar sst_var = data_file.getVar("analysed_sst");
    if (sst_var.isNull()) {
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
    std::cerr << "NetCDF Error [" << input_path.filename() << "]: " << e.what()
              << "\n";
    return std::nullopt;
  }
}
