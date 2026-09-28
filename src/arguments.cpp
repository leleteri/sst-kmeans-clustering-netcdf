#include "arguments.hpp"
#include <iostream>
#include <span>
#include <stdexcept>

Config config;

static std::string usage =
    "Usage: build-sst-dataset [OPTION] [PATH]\n\nFor more "
    "information, try '-h' or '--help'";

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
    } else if (arg == "-l" || arg == "--limit") {
      if (i + 1 >= args.size())
        throw std::runtime_error("--limit requires a value");
      config.limit = std::stoul(argv[++i]);
    } else if (arg == "-o" || arg == "--output") {
      if (i + 1 >= args.size())
        throw std::runtime_error("--output requires a value");
      config.output = args[++i];
    } else if (arg.starts_with("--")) {
      throw std::runtime_error("Unknown argument: " + arg);
    } else {
      positional.push_back(arg);
    }
  }

  return positional;
}
