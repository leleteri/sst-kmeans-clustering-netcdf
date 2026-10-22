#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <cstddef>
#include <string>
#include <vector>

struct InspectOptions {
  bool verbose = false;
  std::vector<std::string> inputs;
};

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

InspectOptions parseInspectArgs(const std::vector<std::string> &args);
BuildOptions parseBuildArgs(const std::vector<std::string> &args);
ClusterOptions parseClusterArgs(const std::vector<std::string> &args);

#endif
