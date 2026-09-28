#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <string>
#include <vector>

struct BuildOptions {
  bool verbose = false;
  size_t limit = 0;
  std::string output = "sst_dataset.nc";
  std::vector<std::string> inputs;
};

struct ClusterOptions {
  bool verbose = false;
  size_t k = 0;
  unsigned seed = 42;
  std::string input;
  std::string output = "clusters.nc";
};

BuildOptions parseBuildArgs(const std::vector<std::string> &args);
ClusterOptions parseClusterArgs(const std::vector<std::string> &args);

#endif
