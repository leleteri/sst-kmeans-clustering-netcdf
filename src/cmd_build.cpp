#include "arguments.hpp"
#include "build/data_validate.hpp"
#include "build/dataset_io.hpp"
#include "build/grid_data.hpp"
#include "build/sst_data.hpp"
#include <cstdio>
#include <cstdlib>
#include <print>
#include <string>
#include <vector>

int runBuild(const std::vector<std::string> &args) {
  BuildOptions opts;
  try {
    opts = parseBuildArgs(args);
  } catch (const std::exception &exception) {
    std::println(stderr, "Argument error: {}.", exception.what());
    return EXIT_FAILURE;
  }

  auto files = collectNcFiles(opts.inputs, opts.verbose);
  if (files.empty()) {
    std::println(stderr, "No .nc files found.");
    return EXIT_FAILURE;
  }
  if (opts.limit > 0 && opts.limit < files.size())
    files.resize(opts.limit);
  if (opts.verbose)
    std::println("Using {} file(s).", files.size());

  auto axis = buildTimeAxis(files, opts.verbose);
  if (!axis)
    return EXIT_FAILURE;

  if (!validateGrid(files, opts.verbose) ||
      !validateStaticMask(files, opts.verbose)) {
    std::println(stderr, "Validation Failed, aborting.");
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
    valid ? std::println("Dataset is valid.")
          : std::println(stderr, "Dataset has missing cells, see above");
  return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
