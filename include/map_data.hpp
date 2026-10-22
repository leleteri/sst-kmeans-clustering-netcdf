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

// Reads the first time step of a (time, latitude, longitude) analysed_sst
// variable. lat_lon_shape holds the expected latitude and longitude counts.
std::optional<std::vector<float>>
readSstFirstStep(netCDF::NcFile &file, const std::string &file_name,
                 const std::vector<size_t> &lat_lon_shape);
std::optional<std::vector<float>>
readSstFirstStep(const std::filesystem::path &input_path,
                 const std::vector<size_t> &lat_lon_shape);

// overload to accept passing opened file
std::optional<LatLon> readLatLon(netCDF::NcFile &file,
                                 const std::string &file_name);
std::optional<std::vector<float>>
readSstValues(netCDF::NcFile &file, const std::string &file_name,
              const std::vector<size_t> &shape);

#endif
