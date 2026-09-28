#include "filename_parse.hpp"
#include <iostream>
#include <optional>

namespace df = std::chrono; // date format

std::optional<df::year_month_day>
parseDateFromFilename(const fs::path &input_path) {
  std::string no_extension_filename = input_path.stem().string();

  if (no_extension_filename.size() < 8)
    return std::nullopt;

  std::string grep_date =
      no_extension_filename.substr(no_extension_filename.size() - 8);

  try {
    int year = std::stoi(grep_date.substr(0, 4));
    int month = std::stoi(grep_date.substr(4, 2));
    int day = std::stoi(grep_date.substr(6, 2));
    return df::year_month_day{df::year{year},
                              df::month{static_cast<unsigned>(month)},
                              df::day{static_cast<unsigned>(day)}};
  } catch (std::exception &exception) {
    std::cerr << "Error: " << exception.what() << ".\n";
    return std::nullopt;
  }
}

long daysSinceEpoch(const df::year_month_day &year_month_day) {
  return df::sys_days{year_month_day}.time_since_epoch().count();
}
