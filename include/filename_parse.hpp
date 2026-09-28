#ifndef FILENAME_PARSER_HPP
#define FILENAME_PARSER_HPP

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

struct TimeAxis {
  std::vector<std::filesystem::path> files;
  std::vector<long> dayOffsets;
};

std::optional<std::chrono::year_month_day>
parseDateFromFilename(const std::filesystem::path &input_path, bool verbose);
long daysSinceEpoch(const std::chrono::year_month_day &year_month_day);
std::vector<std::filesystem::path>
collectNcFiles(const std::vector<std::string> &args, bool verbose);
std::optional<TimeAxis>
buildTimeAxis(const std::vector<std::filesystem::path> &files, bool verbose);

#endif
