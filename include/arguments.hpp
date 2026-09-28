#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <string>
#include <vector>

struct Config {
  bool verbose = false;
  size_t limit = 0;
  std::string output = "sst_dataset.nc";
};

extern Config config;

std::vector<std::string> parseArgs(int argc, char **argv);

#endif
