#include "arguments.hpp"
#include <stdexcept>
#include <string>
#include <vector>

static const std::string &requireValue(const std::vector<std::string> &args,
                                       size_t &i) {
  if (i + 1 >= args.size())
    throw std::runtime_error(args[i] + " requires a value");
  return args[++i];
}

static size_t parseCount(const std::string &flag, const std::string &text) {
  try {
    return std::stoul(text);
  } catch (const std::exception &) {
    throw std::runtime_error(flag + " expects a number, got '" + text + "'");
  }
}

BuildOptions parseBuildArgs(const std::vector<std::string> &args) {
  BuildOptions opts;
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-v" || arg == "--verbose") {
      opts.verbose = true;
    } else if (arg == "-l" || arg == "--limit") {
      opts.limit = parseCount(arg, requireValue(args, i));
    } else if (arg == "-o" || arg == "--output") {
      opts.output = requireValue(args, i);
    } else if (arg.starts_with("-")) {
      throw std::runtime_error("Unknown option for 'build': " + arg);
    } else {
      opts.inputs.push_back(arg);
    }
  }

  if (opts.inputs.empty())
    throw std::runtime_error("build needs at least one file or directory");
  return opts;
}

ClusterOptions parseClusterArgs(const std::vector<std::string> &args) {
  ClusterOptions opts;
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-v" || arg == "--verbose")
      opts.verbose = true;
    else if (arg == "-k" || arg == "--k")
      opts.k = parseCount(arg, requireValue(args, i));
    else if (arg == "--seed")
      opts.seed = static_cast<unsigned>(parseCount(arg, requireValue(args, i)));
    else if (arg == "-o" || arg == "--output")
      opts.output = requireValue(args, i);
    else if (arg.starts_with("-"))
      throw std::runtime_error("Unknown option for 'cluster': " + arg);
    else if (opts.input.empty())
      opts.input = arg;
    else
      throw std::runtime_error("cluster takes exactly one input file");
  }
  if (opts.k == 0)
    throw std::runtime_error("--k is required and must be greater than 0");
  if (opts.input.empty())
    throw std::runtime_error("cluster needs a dataset file");
  return opts;
}
