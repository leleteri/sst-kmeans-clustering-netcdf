#ifndef MAP_DATA_HPP
#define MAP_DATA_HPP

#include <cstddef>
#include <filesystem>
#include <netcdf>
#include <optional>
#include <string>
#include <vector>

struct LatLon {
  std::vector<double> lats;
  std::vector<double> lons;
};

std::optional<LatLon> readLatLon(const std::filesystem::path &input_path);
std::optional<std::vector<float>>
readSstValues(const std::filesystem::path &input_path,
              const std::vector<size_t> &shape);

// overload to accept passing opened file
std::optional<LatLon> readLatLon(netCDF::NcFile &file,
                                 const std::string &file_name);
std::optional<std::vector<float>>
readSstValues(netCDF::NcFile &file, const std::string &file_name,
              const std::vector<size_t> &shape);

#endif
