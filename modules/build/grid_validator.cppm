export module build.grid_validator;

import std;
import nc_utils;
import build.grid_info;

namespace fs = std::filesystem;

export class GridValidator {
public:
  explicit GridValidator(const GridAxes &baseline, double tolerance = 1e-9,
                         bool verbose = false)
      : m_baseline(baseline), m_tolerance(tolerance), m_verbose(verbose) {}

  bool validate(std::span<const fs::path> files) const {
    if (files.empty())
      throw std::invalid_argument("no files to validate.");

    bool all_match = true;
    for (const fs::path &file : files)
      all_match &= matches(file);

    if (m_verbose) {
      std::println("checked {} file(s) against baseline ({}).", files.size(),
                   files.front().filename().string());
      if (all_match)
        std::println("grid is identical across all files.");
    }
    return all_match;
  }

private:
  GridAxes m_baseline;
  double m_tolerance;
  bool m_verbose;

  bool matches(const fs::path &file) const {
    GridAxes axes;
    try {
      axes = GridAxes::read(NcReader(file));
    } catch (const std::exception &e) {
      std::println(std::cerr, "warning: skipping unreadable file: {}",
                   e.what());
      return false;
    }

    bool ok = true;
    if (!m_baseline.sameLatitude(axes, m_tolerance)) {
      std::println(std::cerr, "warning: {}: latitude differs from baseline.",
                   file.string());
      ok = false;
    }
    if (!m_baseline.sameLongitude(axes, m_tolerance)) {
      std::println(std::cerr, "warning: {}: longitude differs from baseline.",
                   file.string());
      ok = false;
    }
    return ok;
  }
};
