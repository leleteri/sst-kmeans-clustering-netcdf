#include "arguments.hpp"
#include "data_validate.hpp"
#include "dataset_io.hpp"
#include "grid_data.hpp"
#include "helper.hpp"
#include "sst_data.hpp"
#include <cstdlib>
#include <iostream>
#include <netcdf>

int main(int argc, char **argv) {
  std::vector<std::string> args;
  try {
    args = parseArgs(argc, argv);
  } catch (const std::exception &exception) {
    std::cerr << "Argument error: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }

  auto files = collectNcFiles(args);
  if (files.empty()) {
    std::cerr << "No .nc files found.\n";
    return EXIT_FAILURE;
  }
  if (config.limit > 0 && config.limit < files.size())
    files.resize(config.limit);
  std::cout << "Using " << files.size() << " file(s).\n";

  auto axis = buildTimeAxis(files);
  if (!axis)
    return EXIT_FAILURE;

  if (!validateGrid(files) || !validateStaticMask(files)) {
    std::cerr << "Validation Failed, aborting.\n";
    return EXIT_FAILURE;
  }

  auto grid = readGridInfo(files.front());
  if (!grid)
    return EXIT_FAILURE;

  auto dataset = buildSstDataSet(*grid, *axis);
  if (!dataset)
    return EXIT_FAILURE;

  bool valid = summarizeDataset(*dataset);
  if (!writeDatasetNc(*dataset, config.output))
    return EXIT_FAILURE;
  std::cout << (valid ? "Dataset is valid.\n"
                      : "Dataset has mising cells, see above.\n");
  return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
