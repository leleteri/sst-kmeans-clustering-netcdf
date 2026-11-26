export module build.command;

import std;
import arguments;
import command;
import build.arguments;
import build.collect_files;
import build.dataset;
import build.dataset_writer;
import build.grid_info;
import build.grid_validator;
import build.time_axis;

// sst build <file|dir>... : daily .nc files -> one (pixel x day) dataset file.
export class BuildCommand final : public Command {
public:
  explicit BuildCommand(const Arguments &args) : m_args(args) {}

  int run() override {
    const bool verbose = m_args.verbose();

    std::vector<std::filesystem::path> files = collectNcFiles(m_args.inputs());
    if (files.empty())
      throw std::runtime_error("no .nc files found.");

    if (m_args.limit() > 0 && m_args.limit() < files.size())
      files.erase(files.begin() + static_cast<std::ptrdiff_t>(m_args.limit()),
                  files.end());
    if (verbose)
      std::println("using {} file(s)", files.size());

    const TimeAxis time_axis(std::move(files), verbose);
    const GridInfo grid(time_axis.files().front());

    if (!GridValidator(grid.axes(), 1e-9, verbose).validate(time_axis.files()))
      throw std::runtime_error("grid validation failed, aborting.");

    const Dataset dataset(grid, time_axis, verbose);
    const Dataset::Summary summary = dataset.summary();
    if (verbose)
      summary.print();

    DatasetWriter(m_args.output(), verbose).write(dataset, grid, time_axis);

    if (summary.complete()) {
      std::println("dataset is valid.");
      return ExitSuccess;
    }
    std::println(std::cerr,
                 "warning: dataset has {} missing cell(s); it was written to "
                 "{} anyway.",
                 summary.missing, m_args.output());
    return ExitIncomplete;
  }

private:
  BuildArgs m_args;
};
