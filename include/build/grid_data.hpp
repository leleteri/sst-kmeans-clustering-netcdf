#ifndef GRID_DATA_HPP
#define GRID_DATA_HPP

#include "map_data.hpp"

#include <filesystem>
#include <netcdf>
#include <optional>
#include <string>
#include <vector>

struct GridInfo {
  LatLon coords;
  std::vector<bool> oceanMask;
  float fill_value;
  std::string crs_wkt;
};

std::optional<GridInfo> readGridInfo(const std::filesystem::path &input_path);

// Reads the _FillValue attribute of the analysed_sst variable.
std::optional<float> readFillValue(netCDF::NcFile &file,
                                    const std::string &file_name);
std::optional<float>
readFillValue(const std::filesystem::path &input_path);

#endif
