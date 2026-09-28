#ifndef GRID_DATA_HPP
#define GRID_DATA_HPP

#include <filesystem>
#include <optional>
#include <vector>

struct LatLon {
  std::vector<double> lats;
  std::vector<double> lons;
};

struct GridInfo {
  LatLon coords;
  std::vector<bool> oceanMask;
  float fill_value;
};

std::optional<GridInfo> readGridInfo(const std::filesystem::path &input_path,
                                     bool verbose);
std::optional<std::vector<float>>
readSstValues(const std::filesystem::path &input_path, size_t expected_size,
              bool verbose);
std::optional<LatLon> readLatLon(const std::filesystem::path &input_path,
                                 bool verbose);
std::optional<std::vector<bool>>
readOceanMask(const std::filesystem::path &input_path, bool verbose);

#endif
