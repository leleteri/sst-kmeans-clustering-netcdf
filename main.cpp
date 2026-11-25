import std;
import arguments;
import build_args;
import grid_info;
import data_validate;
import collect_files;
import dataset;
import time_axis;
import build_output;

namespace fs = std::filesystem;

namespace {

constexpr int printHelp() {
  std::println(std::cerr, "usage: build/sst <subcommand> [options] [file]");
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
      std::println(std::cerr, "error: no .nc files found.");
      return 1;
    }

    if (build_args.limit() > 0 && build_args.limit() < files.size())
      files.resize(build_args.limit());
    std::println("using {} file(s)", files.size());

    TimeAxis time_axis(files);
    if (!time_axis)
      return 1;

    if (!validateGrids(time_axis.files())) {
      std::println(std::cerr, "error: validation failed, aborting.");
      return 1;
    }

    GridInfo grid(time_axis.files().front());
    if (!grid)
      return 1;

    Dataset dataset(grid, time_axis, build_args.verbose());
    if (!dataset.summarize(build_args.verbose()))
      std::println(std::cerr, "warning: dataset contains NaN cells.");

    fs::path output_path("sst_dataset.nc");
    if (!writeDatasetNc(dataset, grid, time_axis, output_path)) {
      std::println(std::cerr, "error:");
      return printHelp();
    }

  } catch (const std::exception &e) {
    std::println(std::cerr, "error: {}", e.what());
    std::println(std::cerr);
    return printHelp();
  }
  return 0;
}
