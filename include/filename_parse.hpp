#ifndef FILENAME_PARSER_HPP
#define FILENAME_PARSER_HPP

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;
namespace df = std::chrono;

struct TimeAxis {
  std::vector<fs::path> files;
  std::vector<long> dayOffsets;
};

std::optional<df::year_month_day>
parseDateFromFilename(const fs::path &input_path, bool verbose);
long daysSinceEpoch(const df::year_month_day &year_month_day);
std::vector<fs::path> collectNcFiles(const std::vector<std::string> &args,
                                     bool verbose);
std::optional<TimeAxis> buildTimeAxis(const std::vector<fs::path> &files,
                                      bool verbose);

#endif
