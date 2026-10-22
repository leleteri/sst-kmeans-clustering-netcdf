#include "arguments.hpp"
#include "build/build_output.hpp"
#include "build/data_validate.hpp"
#include "build/filename_parse.hpp"
#include "build/grid_data.hpp"
#include "build/sst_data.hpp"
#include "commands.hpp"

#include <cstdlib>
#include <print>
#include <string>
#include <vector>

// Exit code used when the dataset was written but is not complete.
constexpr int kExitIncomplete = 2;

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

  if (!validateGrid(axis->files, opts.verbose)) {
    std::println(stderr, "Error: Validation Failed, aborting.");
    return EXIT_FAILURE;
  }

  auto grid = readGridInfo(axis->files.front());
  if (!grid)
    return EXIT_FAILURE;

  // The ocean mask of every file is checked while the data is read, so that
  // each file is only opened and read once.
  auto dataset = buildSstDataSet(*grid, *axis, opts.verbose);
  if (!dataset) {
    std::println(stderr, "Error: Validation Failed, aborting.");
    return EXIT_FAILURE;
  }

  const bool valid = summarizeDataset(*dataset, opts.verbose);
  if (!writeDatasetNc(*dataset, opts.output, opts.verbose))
    return EXIT_FAILURE;

  if (valid) {
    std::println("Dataset is valid.");
    return EXIT_SUCCESS;
  }

  std::println(stderr,
               "Warning: Dataset has missing cells, it was written to {} "
               "anyway{}",
               opts.output, opts.verbose ? ", see above." : ".");
  return kExitIncomplete;
}
