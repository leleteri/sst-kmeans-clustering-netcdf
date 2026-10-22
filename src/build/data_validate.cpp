#include "build/data_validate.hpp"
#include "build/grid_data.hpp"
#include "map_data.hpp"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <print>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool sameAxis(const std::vector<double> &a, const std::vector<double> &b,
              double tolerance) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i) {
    // NaN coordinates never compare equal, so they are rejected explicitly.
    if (std::isnan(a[i]) || std::isnan(b[i]))
      return false;
    if (std::abs(a[i] - b[i]) > tolerance)
      return false;
  }
  return true;
}

} // namespace

bool validateGrid(const std::vector<fs::path> &files, bool verbose,
                  double tolerance) {
  if (files.empty()) {
    std::println(stderr, "Error: No files given.");
    return false;
  }

  auto baseline_opt = readLatLon(files.front());
  if (!baseline_opt) {
    std::println(stderr, "Error: Failed to read the baseline coordinates "
                         "from {}.",
                 files.front().string());
    return false;
  }
  const LatLon &baseline = *baseline_opt;

  bool all_match = true;
  size_t files_checked = 0;

  for (size_t i = 1; i < files.size(); ++i) {
    auto grid_opt = readLatLon(files[i]);
    if (!grid_opt) {
      std::println(stderr, "Warning: Skipping unreadable file: {}.",
                   files[i].string());
      all_match = false;
      continue;
    }
    ++files_checked;

    if (!sameAxis(baseline.lats, grid_opt->lats, tolerance)) {
      std::println(stderr, "Warning: {}: latitude differs from baseline.",
                   files[i].string());
      all_match = false;
    }
    if (!sameAxis(baseline.lons, grid_opt->lons, tolerance)) {
      std::println(stderr, "Warning: {}: longitude differs from baseline.",
                   files[i].string());
      all_match = false;
    }
  }

  if (verbose) {
    std::println("Checked {} file(s) against baseline ({}).", files_checked,
                 files.front().filename().string());
    if (all_match)
      std::println("Grid is identical across all files.");
    else
      std::println(stderr,
                   "Warning: Grid is NOT identical, see differences above.");
  }

  return all_match;
}
