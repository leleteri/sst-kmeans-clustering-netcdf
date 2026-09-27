#include "sst_data.hpp"
#include "filename_parse.hpp"
#include "grid_data.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <netcdf>
#include <optional>
#include <vector>

std::optional<SstDataset> buildSstDataSet(const GridInfo &grid,
                                          const TimeAxis &time_axis) {
  size_t num_rows =
      std::count(grid.oceanMask.begin(), grid.oceanMask.end(), true);
  size_t num_days = time_axis.dayOffsets.size();
  size_t grid_size = grid.lats.size() * grid.lons.size();

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
    auto sstOpt = readSstValues(time_axis.files[day], grid_size);
    if (!sstOpt) {
      std::cerr << "Failed to read " << time_axis.files[day]
                << ", skipping.\nn";
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
