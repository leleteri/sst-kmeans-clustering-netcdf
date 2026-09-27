#include "arguments.hpp"
#include <span>
#include <stdexcept>

Config config;

std::vector<std::string> parseArgs(int argc, char **argv) {
  const std::span<char *> args{argv, static_cast<std::size_t>(argc)};
  std::vector<std::string> positional;

  if (args.size() < 2) {
    throw std::runtime_error("Error: no arguments were passed");
  }

  for (size_t i = 1; i < args.size(); ++i) {
    std::string arg = args[i];

    if (arg == "-v" || arg == "--verbose") {
      config.verbose = true;
    } else if (arg == "--show-missing") {
      config.showMissingValues = true;
    } else if (arg == "--export-as-csv") {
      config.outputCsv = true;
    } else if (arg == "-m" || arg == "--method") {
      if (i + 1 >= args.size())
        throw std::runtime_error(arg + " requires a value");
      // Need to implement validation later
      config.method = args[++i];
    } else if (arg.starts_with("--")) {
      throw std::runtime_error("Unknown option: " + arg);
    } else {
      positional.push_back(arg);
    }
  }

  if (config.method.empty())
    throw std::runtime_error("-m or --method is required");

  return positional;
}
