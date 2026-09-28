#include "arguments.hpp"
#include "grid_data.hpp"
#include <cstdlib>
#include <iostream>

static bool sameAxis(const std::vector<double> &a, const std::vector<double> &b,
                     double tolerance) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i)
    if (std::abs(a[i] - b[i]) > tolerance)
      return false;
  return true;
}

bool validateGrid(const std::vector<fs::path> &files, double tolerance = 1e-9) {
  if (files.empty()) {
    std::cerr << "No files given.\n";
    return false;
  }

  auto baseline_opt = readLatLon(files.front());
  if (!baseline_opt) {
    std::cerr << "Failed to read baseline mask from " << files.front() << "\n";
    return false;
  }
  const LatLon &baseline = *baseline_opt;

  bool all_match = true;
  size_t files_checked = 0;

  for (size_t i = 1; i < files.size(); ++i) {
    auto grid_opt = readLatLon(files[i]);
    if (!grid_opt) {
      std::cerr << "Skipping unreadable file: " << files[i] << "\n";
      all_match = false;
      continue;
    }
    ++files_checked;

    if (!sameAxis(baseline.lats, grid_opt->lats, tolerance)) {
      if (config.verbose)
        std::cout << files[i].filename()
                  << ": latitude differs from baseline\n";
      all_match = false;
    }
    if (!sameAxis(baseline.lons, grid_opt->lons, tolerance)) {
      if (config.verbose)
        std::cout << files[i].filename()
                  << ": longitude differs from baseline\n";
      all_match = false;
    }
  }

  if (config.verbose) {
    std::cout << "Checked " << files_checked << " file(s) against baseline ("
              << files.front().filename() << ").\n";
    std::cout << (all_match
                      ? "Grid is identical across all files.\n"
                      : "Grid is NOT identical, see differences above.\n");
  };

  return all_match;
}

bool validateStaticMask(const std::vector<fs::path> &files) {
  if (files.empty()) {
    std::cerr << "No files given.\n";
    return false;
  }

  auto baseline_opt = readOceanMask(files.front());
  if (!baseline_opt) {
    std::cerr << "Failed to read baseline mask from " << files.front() << "\n";
    return false;
  }

  const std::vector<bool> baseline = *baseline_opt;

  bool all_match = true;
  size_t files_checked = 0;

  for (size_t i = 1; i < files.size(); ++i) {
    auto mask_opt = readOceanMask(files[i]);
    if (!mask_opt) {
      std::cerr << "Skipping unreadable file: " << files[i] << "\n";
      continue;
    }
    const std::vector<bool> &mask = *mask_opt;
    ++files_checked;

    if (mask.size() != baseline.size()) {
      std::cerr << "Grid size mismatch in " << files[i].filename() << "\n";
      all_match = false;
      continue;
    }

    size_t diff_count = 0;
    for (size_t i = 0; i < baseline.size(); ++i) {
      if (mask[i] != baseline[i])
        ++diff_count;
    }

    if (diff_count > 0) {
      if (config.verbose)
        std::cout << files[i].filename() << ": " << diff_count
                  << " pixel(s) differ from baseline mask\n";
      all_match = false;
    }
  }

  if (config.verbose) {
    std::cout << "Checked " << files_checked << " file(s) against baseline ("
              << files.front().filename() << ").\n";
    std::cout << (all_match ? "Mask is static across all files.\n"
                            : "Mask is NOT static — see differences above.\n");
  }

  return all_match;
}
