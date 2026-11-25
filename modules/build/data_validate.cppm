export module data_validate;

import std;
import grid_info;

namespace fs = std::filesystem;

namespace {

bool sameAxis(const std::vector<double> &a, const std::vector<double> &b,
              double tolerance) {
  if (a.size() != b.size())
    return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::isnan(a[i]) || std::isnan(b[i]))
      return false;
    if (std::abs(a[i] - b[i]) > tolerance)
      return false;
  }
  return true;
}

} // namespace

export bool validateGrids(const std::vector<fs::path> &files,
                          double tolerance = 1e-9) {
  if (files.empty()) {
    throw std::invalid_argument("no files given.");
    return false;
  }

  GridInfo baseline(files.front());

  bool grid_match = true;
  bool mask_static = true;
  std::size_t mask_diffs = 0;
  std::size_t files_checked = 0;

  for (std::size_t i = 1; i < files.size(); ++i) {
    GridInfo grid(files[i]);
    if (!grid) {
      std::println(std::cerr, "warning: skipping unreadable file: {}.",
                   files[i].string());
      grid_match = false;
      continue;
    }
    ++files_checked;

    if (!sameAxis(baseline.latitude(), grid.latitude(), tolerance)) {
      std::println(std::cerr, "warning: {}: latitude differs from baseline.",
                   files[i].string());
      grid_match = false;
    }
    if (!sameAxis(baseline.longitude(), grid.longitude(), tolerance)) {
      std::println(std::cerr, "warning: {}: longitude differs from baseline.",
                   files[i].string());
      grid_match = false;
    }

    if (baseline.oceanMask().size() != grid.oceanMask().size()) {
      mask_diffs =
          std::max(baseline.oceanMask().size(), grid.oceanMask().size());
      std::println(std::cerr,
                   "warning: {}: {} pixel(s) differ from the baseline ocean "
                   "mask.",
                   files[i].string(), mask_diffs);
      mask_static = false;
    } else {
      for (std::size_t j = 0; j < baseline.oceanMask().size(); ++j) {
        if (baseline.oceanMask()[j] != grid.oceanMask()[j])
          ++mask_diffs;
      }
      if (mask_diffs > 0) {
        std::println(std::cerr,
                     "warning: {}: {} pixel(s) differ from the baseline ocean "
                     "mask.",
                     files[i].string(), mask_diffs);
        mask_static = false;
      }
    }
  }

  std::println("checked {} file(s) against baseline ({}).", files_checked,
               files.front().filename().string());
  if (grid_match)
    std::println("grid is identical across all files.");
  else
    std::println(std::cerr,
                 "warning: grid is NOT identical, see differences above.");

  if (mask_static)
    std::println("mask is static across all files");
  else
    std::println(std::cerr,
                 "warning: mask is NOT static, see differences above");

  return (grid_match && mask_static);
}
