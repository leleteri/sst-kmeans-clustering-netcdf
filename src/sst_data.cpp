#include "sst_data.hpp"
#include "filename_parse.hpp"
#include "grid_data.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <netcdf>
#include <optional>
#include <vector>

std::optional<SstDataset>
buildSstDataSet(const GridInfo &grid, const TimeAxis &time_axis, bool verbose) {
  size_t num_rows =
      std::count(grid.oceanMask.begin(), grid.oceanMask.end(), true);
  size_t num_days = time_axis.dayOffsets.size();
  size_t grid_size = grid.coords.lats.size() * grid.coords.lons.size();

  SstDataset dataset;
  dataset.grid = grid;
  dataset.time = time_axis;
  dataset.matrix.assign(num_rows * num_days,
                        std::numeric_limits<float>::quiet_NaN());

  std::vector<size_t> grid_index_to_row(grid_size, SIZE_MAX);
  size_t rowCounter = 0;
  for (size_t i = 0; i < grid_size; ++i) {
    if (grid.oceanMask[i]) {
      grid_index_to_row[i] = rowCounter++;
    }
  }

  for (size_t day = 0; day < time_axis.files.size(); ++day) {
    auto sstOpt = readSstValues(time_axis.files[day], grid_size, verbose);
    if (!sstOpt) {
      if (verbose)
        std::cerr << "Failed to read " << time_axis.files[day]
                  << ", skipping.\n";
      continue;
    }
    const std::vector<float> &sst = *sstOpt;

    for (size_t grid_index = 0; grid_index < grid_size; ++grid_index) {
      size_t row = grid_index_to_row[grid_index];
      if (row == SIZE_MAX)
        continue;
      dataset.matrix[row * num_days + day] = sst[grid_index];
    }
  }

  return dataset;
}

bool summarizeDataset(const SstDataset &dataset, bool verbose) {
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
  if (verbose)
    std::cout << "Rows (ocean pixels): " << num_rows << "\n"
              << "Columns (days):      " << num_days << "\n"
              << "NaN cells:           " << nan_count << "\n"
              << "Min / Max / Mean:    " << minV << " / " << maxV << " / "
              << (valid ? sum / valid : 0.0) << "\n";
  return nan_count == 0;
}
