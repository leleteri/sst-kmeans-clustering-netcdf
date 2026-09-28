#include "arguments.hpp"
#include "csv_write.hpp"
#include "kmeans.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <netcdf>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
  std::vector<std::string> files;

  fs::path inputPath;

  if (config.outputCsv) {
    // output = convertNcToCsv(output)
  }

  return EXIT_SUCCESS;
}
