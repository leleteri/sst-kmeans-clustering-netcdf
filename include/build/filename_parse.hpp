#ifndef FILENAME_PARSE_HPP
#define FILENAME_PARSE_HPP

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

struct TimeAxis {
  std::vector<std::filesystem::path> files;
  std::vector<long> dayOffsets;
};

// Parses the YYYYMMDD suffix of a file name.
std::optional<std::chrono::year_month_day>
parseDateFromFilename(const std::filesystem::path &input_path);

// Number of days between 1970-01-01 and the given date.
long daysSinceEpoch(const std::chrono::year_month_day &year_month_day);

// Collects the netCDF files of the given files and directories.
std::vector<std::filesystem::path>
collectNcFiles(const std::vector<std::string> &args);

// Builds the time axis, ordered chronologically.
std::optional<TimeAxis>
buildTimeAxis(const std::vector<std::filesystem::path> &files, bool verbose);

#endif
