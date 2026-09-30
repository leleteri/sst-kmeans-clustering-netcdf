#ifndef CLUSTER_DATA_HPP
#define CLUSTER_DATA_HPP

#include <filesystem>
#include <optional>
#include <vector>

struct ClusterInput {
  std::vector<double> lats;
  std::vector<double> lons;
  std::vector<long> days;
  std::vector<float> sst;
  size_t num_rows;
  size_t num_cols;
};

std::optional<ClusterInput> readClusterInput(const std::filesystem::path &path);

#endif
