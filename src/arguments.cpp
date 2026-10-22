#include "arguments.hpp"
#include <charconv>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

const std::string &requireValue(const std::vector<std::string> &args,
                                size_t &i) {
  if (i + 1 >= args.size())
    throw std::runtime_error(args[i] + " requires a value");
  return args[++i];
}

// Parses a non-negative decimal number. The whole token must be consumed and
// a negative value is rejected, so that "-1" cannot silently wrap around.
size_t parseCount(const std::string &flag, const std::string &text) {
  if (text.empty())
    throw std::runtime_error(flag + " expects a number, got an empty value");

  size_t parsed = 0;
  const char *begin = text.data();
  const char *end = text.data() + text.size();
  const std::from_chars_result result = std::from_chars(begin, end, parsed);
  if (result.ec == std::errc::invalid_argument ||
      result.ec == std::errc::result_out_of_range || result.ptr != end) {
    throw std::runtime_error(flag + " expects a number, got '" + text + "'");
  }
  return parsed;
}

// Parses a number that has to fit in an unsigned int.
unsigned parseUnsigned(const std::string &flag, const std::string &text) {
  const size_t parsed = parseCount(flag, text);
  if (parsed > std::numeric_limits<unsigned>::max())
    throw std::runtime_error(
        flag + " expects a value between 0 and " +
        std::to_string(std::numeric_limits<unsigned>::max()));
  return static_cast<unsigned>(parsed);
}

} // namespace

InspectOptions parseInspectArgs(const std::vector<std::string> &args) {
  InspectOptions opts;
  for (const std::string &arg : args) {
    if (arg == "-v" || arg == "--verbose") {
      opts.verbose = true;
    } else if (arg.starts_with("-")) {
      throw std::runtime_error("Unknown option for 'inspect': " + arg);
    } else {
      opts.inputs.push_back(arg);
    }
  }

  if (opts.inputs.empty())
    throw std::runtime_error("inspect needs at least one .nc file");
  return opts;
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
    else if (arg == "-k" || arg == "--k-value")
      opts.k = parseCount(arg, requireValue(args, i));
    else if (arg == "--seed")
      opts.seed = parseUnsigned(arg, requireValue(args, i));
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
    throw std::runtime_error(
        "--k-value is required and must be greater than 0");
  if (opts.input.empty())
    throw std::runtime_error("cluster needs a dataset file");
  return opts;
}
