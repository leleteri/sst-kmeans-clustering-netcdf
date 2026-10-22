#include "arguments.hpp"
#include "build/build_output.hpp"
#include "build/data_validate.hpp"
#include "build/filename_parse.hpp"
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
    std::println(stderr, "Error: Argument error: {}.", exception.what());
    return EXIT_FAILURE;
  }

  auto files = collectNcFiles(opts.inputs);
  if (files.empty()) {
    std::println(stderr, "Error: No .nc files found.");
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
    std::println(stderr, "Error: Validation Failed, aborting.");
    return EXIT_FAILURE;
  }

  auto grid = readGridInfo(files.front());
  if (!grid)
    return EXIT_FAILURE;

  auto dataset = buildSstDataSet(*grid, *axis);
  if (!dataset)
    return EXIT_FAILURE;

  bool valid = summarizeDataset(*dataset, opts.verbose);
  if (!writeDatasetNc(*dataset, opts.output, opts.verbose))
    return EXIT_FAILURE;
  if (valid) {
    std::println("Dataset is valid.");
  } else {
    std::println(stderr, "Warning: Dataset has missing cells{}",
                 opts.verbose ? ", see above." : ".");
  }
  return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
