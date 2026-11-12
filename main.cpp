#include <print>

import arguments;
import build_args;
import grid_info;
import data_validate;
import collect_files;
import dataset;
import time_axis;

namespace {

constexpr int printHelp() {
  std::println(stderr, "usage: build/sst <subcommand> [options] [file]");
  return 1;
}

} // namespace

int main(int argc, char **argv) {
  try {
    Arguments args(argc, argv);
    BuildArgs build_args;
    if (args.subcommand() == "build") {
      build_args = BuildArgs(args.inputs(), args.options());
    }

    auto files = collectNcFiles(args.inputs());
    if (files.empty()) {
      std::println(stderr, "error: no .nc files found.");
      return 1;
    }

    if (build_args.limit() > 0 && build_args.limit() < files.size())
      files.resize(build_args.limit());
    std::println("using {} file(s)", files.size());

    TimeAxis time_axis(files);
    if (!time_axis)
      return 1;

    if (!validateGrids(time_axis.files())) {
      std::println(stderr, "error: validation failed, aborting.");
      return 1;
    }

    GridInfo grid(time_axis.files().front());
    if (!grid)
      return 1;

    Dataset dataset(grid, time_axis, build_args.verbose());
    if (!dataset.summarize(build_args.verbose()))
      std::println(stderr, "warning: dataset contains NaN cells.");
  } catch (const std::exception &e) {
    std::println(stderr, "error: {}", e.what());
    std::println(stderr);
    return printHelp();
  }
  return 0;
}
