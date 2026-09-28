#include "filename_parse.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace fs = std::filesystem;
namespace df = std::chrono; // date format

std::optional<df::year_month_day>
parseDateFromFilename(const fs::path &input_path, bool verbose) {
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
    if (verbose)
      std::cerr << "Error: " << exception.what() << ".\n";
    return std::nullopt;
  }
}

long daysSinceEpoch(const df::year_month_day &year_month_day) {
  return df::sys_days{year_month_day}.time_since_epoch().count();
}

std::vector<fs::path> collectNcFiles(const std::vector<std::string> &args,
                                     bool verbose) {
  std::vector<fs::path> files;
  for (const auto &arg : args) {
    fs::path p{arg};
    if (fs::is_directory(p)) {
      for (const auto &entry : fs::directory_iterator(p)) {
        if (entry.path().extension() == ".nc")
          files.push_back(entry.path());
      }
    } else if (fs::is_regular_file(p)) {
      files.push_back(p);
    } else {
      if (verbose)
        std::cerr << "Skipping (not a file or directory): " << p << "\n";
    }
  }
  // YYYYMMDD in the name means string order is chronological order
  std::sort(files.begin(), files.end(),
            [](const fs::path &a, const fs::path &b) {
              return a.filename() < b.filename();
            });
  return files;
}

std::optional<TimeAxis> buildTimeAxis(const std::vector<fs::path> &files,
                                      bool verbose) {
  TimeAxis axis;
  for (const auto &f : files) {
    auto date = parseDateFromFilename(f, verbose);
    if (!date || !date->ok()) {
      if (verbose)
        std::cerr << "Could not parse a date from " << f.filename() << "\n";
      return std::nullopt;
    }
    axis.files.push_back(f);
    axis.dayOffsets.push_back(daysSinceEpoch(*date));
  }

  size_t gaps = 0;
  for (size_t i = 1; i < axis.dayOffsets.size(); ++i) {
    long delta = axis.dayOffsets[i] - axis.dayOffsets[i - 1];
    if (delta != 1) {
      if (verbose)
        std::cout << "Date gap/duplicate: " << axis.files[i - 1].filename()
                  << " -> " << axis.files[i].filename() << " (" << delta
                  << " days)\n";
      ++gaps;
    }
  }
  if (verbose)
    std::cout << "Time axis: " << axis.files.size() << " files, " << gaps
              << " irregular step(s).\n";
  return axis;
}
