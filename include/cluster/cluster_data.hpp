#ifndef CLUSTER_DATA_HPP
#define CLUSTER_DATA_HPP

#include "map_data.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

struct ClusterInput {
  LatLon coords;
  std::vector<long> days;
  std::vector<float> sst;
  size_t num_rows = 0;
  size_t num_cols = 0;
};

std::optional<ClusterInput> readClusterInput(const std::filesystem::path &path);

#endif
