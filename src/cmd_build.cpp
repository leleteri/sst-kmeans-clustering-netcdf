#include "arguments.hpp"
#include "data_validate.hpp"
#include "dataset_io.hpp"
#include "grid_data.hpp"
#include "sst_data.hpp"
#include <cstdlib>
#include <iostream>
#include <netcdf>

int runBuild(const std::vector<std::string> &args) {
  BuildOptions opts;
  try {
    opts = parseBuildArgs(args);
  } catch (const std::exception &exception) {
    std::cerr << "Argument error: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }

  auto files = collectNcFiles(opts.inputs, opts.verbose);
  if (files.empty()) {
    std::cerr << "No .nc files found.\n";
    return EXIT_FAILURE;
  }
  if (opts.limit > 0 && opts.limit < files.size())
    files.resize(opts.limit);
  if (opts.verbose)
    std::cout << "Using " << files.size() << " file(s).\n";

  auto axis = buildTimeAxis(files, opts.verbose);
  if (!axis)
    return EXIT_FAILURE;

  if (!validateGrid(files, opts.verbose) ||
      !validateStaticMask(files, opts.verbose)) {
    std::cerr << "Validation Failed, aborting.\n";
    return EXIT_FAILURE;
  }

  auto grid = readGridInfo(files.front(), opts.verbose);
  if (!grid)
    return EXIT_FAILURE;

  auto dataset = buildSstDataSet(*grid, *axis, opts.verbose);
  if (!dataset)
    return EXIT_FAILURE;

  bool valid = summarizeDataset(*dataset, opts.verbose);
  if (!writeDatasetNc(*dataset, opts.output, opts.verbose))
    return EXIT_FAILURE;
  if (opts.verbose)
    std::cout << (valid ? "Dataset is valid.\n"
                        : "Dataset has mising cells, see above.\n");
  return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
