#include "filename_parse.hpp"

namespace df = std::chrono; // date format

std::optional<df::year_month_day>
parseDataFromFileName(const fs::path &input_path) {
  std::string no_extension_filename = input_path.stem().string();

  if (no_extension_filename.size() < 8)
    return std::nullopt;

  std::string grep_date =
      no_extension_filename.substr(no_extension_filename.size() - 8);
  int year = std::stoi(grep_date.substr(0, 4));
  int month = std::stoi(grep_date.substr(4, 2));
  int day = std::stoi(grep_date.substr(6, 2));

  return df::year_month_day{df::year{year},
                            df::month{static_cast<unsigned>(month)},
                            df::day{static_cast<unsigned>(day)}};
}

long daySinceEpoch(const df::year_month_day &year_month_day) {
  return df::sys_days{year_month_day}.time_since_epoch().count();
}
