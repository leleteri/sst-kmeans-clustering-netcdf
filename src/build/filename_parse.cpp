#include "build/filename_parse.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <print>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
namespace df = std::chrono; // date format

std::optional<df::year_month_day>
parseDateFromFilename(const fs::path &input_path) {
  const std::string stem = input_path.stem().string();

  if (stem.size() < 8)
    return std::nullopt;

  const std::string grep_date = stem.substr(stem.size() - 8);

  int year = 0;
  int month = 0;
  int day = 0;
  try {
    size_t pos = 0;
    year = std::stoi(grep_date.substr(0, 4), &pos);
    month = std::stoi(grep_date.substr(4, 2), &pos);
    day = std::stoi(grep_date.substr(6, 2), &pos);
  } catch (const std::exception &exception) {
    std::println(stderr, "Error: {}: {}", input_path.filename().string(),
                 exception.what());
    return std::nullopt;
  }

  // std::chrono::month and std::chrono::day have undefined behaviour for
  // values outside their valid range, so the range is checked up front.
  if (month < 1 || month > 12 || day < 1 || day > 31) {
    std::println(stderr, "Error: {}: Invalid date {}.", input_path.filename().string(),
                 grep_date);
    return std::nullopt;
  }

  const df::year_month_day date{df::year{year},
                                df::month{static_cast<unsigned>(month)},
                                df::day{static_cast<unsigned>(day)}};
  if (!date.ok()) {
    std::println(stderr, "Error: {}: Invalid date {}.", input_path.filename().string(),
                 grep_date);
    return std::nullopt;
  }
  return date;
}

long daysSinceEpoch(const df::year_month_day &year_month_day) {
  return df::sys_days{year_month_day}.time_since_epoch().count();
}

std::vector<fs::path> collectNcFiles(const std::vector<std::string> &args) {
  std::vector<fs::path> files;
  for (const auto &arg : args) {
    fs::path p{arg};
    if (fs::is_directory(p)) {
      for (const auto &entry : fs::directory_iterator(p)) {
        if (entry.path().extension() == ".nc")
          files.push_back(entry.path());
      }
    } else if (fs::is_regular_file(p)) {
      // Explicitly named files are always used, but a non netCDF extension is
      // worth a warning because directory scanning only picks up ".nc" files.
      if (p.extension() != ".nc") {
        std::println(stderr, "Warning: {} does not have a .nc extension.",
                     p.string());
      }
      files.push_back(p);
    } else {
      std::println(stderr, "Warning: Skipping (not a file or directory): {}.",
                   p.string());
    }
  }
  // Deterministic order, so that the baseline file does not depend on the
  // order in which the file system reports the directory entries.
  std::sort(files.begin(), files.end(),
            [](const fs::path &a, const fs::path &b) {
              return a.filename() < b.filename();
            });
  return files;
}

std::optional<TimeAxis> buildTimeAxis(const std::vector<fs::path> &files,
                                      bool verbose) {
  std::vector<std::pair<fs::path, long>> entries;
  entries.reserve(files.size());

  for (const auto &f : files) {
    auto date = parseDateFromFilename(f);
    if (!date) {
      std::println(stderr, "Error: Could not parse a date from {}.",
                   f.string());
      return std::nullopt;
    }
    entries.emplace_back(f, daysSinceEpoch(*date));
  }

  // The time axis is ordered by date, not by file name: the file name only
  // sorts chronologically when every file shares the same prefix.
  std::stable_sort(entries.begin(), entries.end(),
                   [](const auto &a, const auto &b) {
                     return a.second < b.second;
                   });

  TimeAxis axis;
  axis.files.reserve(entries.size());
  axis.dayOffsets.reserve(entries.size());
  for (const auto &[file, offset] : entries) {
    axis.files.push_back(file);
    axis.dayOffsets.push_back(offset);
  }

  size_t gaps = 0;
  for (size_t i = 1; i < axis.dayOffsets.size(); ++i) {
    const long delta = axis.dayOffsets[i] - axis.dayOffsets[i - 1];
    if (delta == 0) {
      std::println(stderr, "Error: Duplicate date: {} and {}.",
                   axis.files[i - 1].string(), axis.files[i].string());
      return std::nullopt;
    }
    if (delta != 1) {
      std::println(stderr, "Warning: Date gap: {} -> {} ({} days).",
                   axis.files[i - 1].string(), axis.files[i].string(), delta);
      ++gaps;
    }
  }
  if (verbose)
    std::println("Time axis: {} files, {} irregular step(s).",
                 axis.files.size(), gaps);
  return axis;
}
