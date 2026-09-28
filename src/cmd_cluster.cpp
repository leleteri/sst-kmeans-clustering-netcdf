#include "arguments.hpp"
#include <cstdlib>
#include <iostream>
#include <netcdf>
#include <string>
#include <vector>

int runCluster(const std::vector<std::string> &args) {
  ClusterOptions opts;
  try {
    opts = parseClusterArgs(args);
  } catch (const std::exception &exception) {
    std::cerr << "Argument error: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
