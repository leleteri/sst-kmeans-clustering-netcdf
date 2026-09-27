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

std::optional<std::chrono::year_month_day>
parseDataFromFileName(const fs::path input_path);
long daySinceEpoch(const df::year_month_day &year_month_day);

#endif
