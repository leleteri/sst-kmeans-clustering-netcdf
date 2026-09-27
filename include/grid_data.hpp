#ifndef GRID_DATA_HPP
#define GRID_DATA_HPP

#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

struct GridInfo {
  std::vector<double> lats;
  std::vector<double> lons;
  std::vector<bool> oceanMask;
  float fillValue;
};

std::optional<GridInfo> readGridInfo(const fs::path &input_path);
std::optional<std::vector<float>> readSstValues(const fs::path &input_path,
                                                size_t expected_size);

#endif
