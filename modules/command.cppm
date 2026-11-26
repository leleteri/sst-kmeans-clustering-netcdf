export module command;

import std;

export inline constexpr int ExitSuccess = 0;
export inline constexpr int ExitError = 1;
export inline constexpr int ExitIncomplete = 2;

export inline constexpr std::string_view Usage =
    R"(usage: sst <command> [options] [inputs]

commands:
  inspect <file.nc>...       print dimensions, variables and attributes
  build   <file|dir>...      read daily .nc files and write the dataset file
  cluster <dataset.nc>       k-means clustering of the monthly climatology
  help                       show this message

options (-name=value or --name=value):
  inspect   -v, -verbose            also print global attributes
  build     -v, -verbose            print progress and a dataset summary
            -l, -limit=N            use only the first N files (by name)
            -o, -output=FILE        output file          [sst_dataset.nc]
  cluster   -v, -verbose            print cluster sizes and centers
            -k, -k-value=N          number of clusters   (required)
            -seed=N                 random seed          [42]
            -o, -output=FILE        output file          [clusters.nc]

exit codes:
  0  success
  1  error
  2  the dataset was written but contains missing cells)";

// One subcommand. Each concrete command parses its own arguments in the
// constructor (throwing ArgumentError) and does the work in run().
export class Command {
public:
  Command() = default;
  Command(const Command &) = delete;
  Command &operator=(const Command &) = delete;
  virtual ~Command() = default;

  // Returns the process exit code. Failures are reported by throwing.
  virtual int run() = 0;
};

export class HelpCommand final : public Command {
public:
  int run() override {
    std::println("{}", Usage);
    return ExitSuccess;
  }
};
