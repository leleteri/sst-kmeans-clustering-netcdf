#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <string>
#include <vector>

struct Config {
  bool verbose = false;
  bool showMissingValues = false;
  bool outputCsv = false;
  std::string method;
};

extern Config config;

std::vector<std::string> parseArgs(int argc, char **argv);

#endif
