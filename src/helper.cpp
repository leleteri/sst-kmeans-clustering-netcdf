#include "helper.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

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
      files.push_back(p);
    } else {
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

std::optional<TimeAxis> buildTimeAxis(const std::vector<fs::path> &files) {
  TimeAxis axis;
  for (const auto &f : files) {
    auto date = parseDateFromFilename(f);
    if (!date || !date->ok()) {
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
      std::cout << "Date gap/duplicate: " << axis.files[i - 1].filename()
                << " -> " << axis.files[i].filename() << " (" << delta
                << " days)\n";
      ++gaps;
    }
  }
  std::cout << "Time axis: " << axis.files.size() << " files, " << gaps
            << " irregular step(s).\n";
  return axis;
}

bool summarizeDataset(const SstDataset &dataset) {
  const size_t num_days = dataset.time.dayOffsets.size();
  const size_t num_rows = num_days ? dataset.matrix.size() / num_days : 0;

  size_t nan_count = 0;
  double sum = 0.0;
  float minV = std::numeric_limits<float>::max();
  float maxV = std::numeric_limits<float>::lowest();

  for (float v : dataset.matrix) {
    if (std::isnan(v)) {
      ++nan_count;
      continue;
    }
    sum += v;
    minV = std::min(minV, v);
    maxV = std::max(maxV, v);
  }

  const size_t valid = dataset.matrix.size() - nan_count;
  std::cout << "Rows (ocean pixels): " << num_rows << "\n"
            << "Columns (days):      " << num_days << "\n"
            << "NaN cells:           " << nan_count << "\n"
            << "Min / Max / Mean:    " << minV << " / " << maxV << " / "
            << (valid ? sum / valid : 0.0) << "\n";
  return nan_count == 0;
}
