#ifndef GRID_DATA_HPP
#define GRID_DATA_HPP

#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

struct LatLon {
  std::vector<double> lats;
  std::vector<double> lons;
};

struct GridInfo {
  LatLon coords;
  std::vector<bool> oceanMask;
  float fill_value;
};

std::optional<GridInfo> readGridInfo(const fs::path &input_path);
std::optional<std::vector<float>> readSstValues(const fs::path &input_path,
                                                size_t expected_size);
std::optional<LatLon> readLatLon(const fs::path &input_path);
std::optional<std::vector<bool>> readOceanMask(const fs::path &input_path);

#endif
