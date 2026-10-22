#include "build/sst_data.hpp"
#include "build/filename_parse.hpp"
#include "build/grid_data.hpp"
#include "map_data.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <netcdf>
#include <optional>
#include <print>
#include <string>
#include <vector>

namespace {

// Counts the pixels whose ocean mask differs from the baseline.
size_t countMaskDifferences(const std::vector<bool> &baseline,
                            const std::vector<bool> &other) {
  if (baseline.size() != other.size())
    return std::max(baseline.size(), other.size());

  size_t differences = 0;
  for (size_t i = 0; i < baseline.size(); ++i)
    if (baseline[i] != other[i])
      ++differences;
  return differences;
}

} // namespace

std::optional<SstDataset> buildSstDataSet(const GridInfo &grid,
                                          const TimeAxis &time_axis,
                                          bool verbose) {
  const size_t num_rows =
      std::count(grid.oceanMask.begin(), grid.oceanMask.end(), true);
  const size_t num_days = time_axis.dayOffsets.size();
  const size_t num_lats = grid.coords.lats.size();
  const size_t num_lons = grid.coords.lons.size();
  const size_t grid_size = num_lats * num_lons;

  if (grid_size != grid.oceanMask.size()) {
    std::println(stderr,
                 "Error: Ocean mask size does not match the coordinate grid.");
    return std::nullopt;
  }

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
    const std::filesystem::path &path = time_axis.files[day];
    const std::string file_name = path.filename().string();

    std::vector<float> sst;
    float fill_value = grid.fill_value;
    try {
      netCDF::NcFile file(path.string(), netCDF::NcFile::read);
      auto values = readSstFirstStep(file, file_name, {num_lats, num_lons});
      if (!values)
        return std::nullopt;
      sst = std::move(*values);

      auto fill = readFillValue(file, file_name);
      if (!fill)
        return std::nullopt;
      fill_value = *fill;
    } catch (const netCDF::exceptions::NcException &exception) {
      std::println(stderr, "Error: {}: NetCDF: {}.", file_name,
                   exception.what());
      return std::nullopt;
    }

    // The ocean mask must be identical in every file.
    std::vector<bool> mask(sst.size());
    for (size_t i = 0; i < sst.size(); ++i)
      mask[i] = (sst[i] != fill_value);

    const size_t differences =
        countMaskDifferences(grid.oceanMask, mask);
    if (differences > 0) {
      std::println(stderr,
                   "Error: {}: {} pixel(s) differ from the baseline ocean "
                   "mask.",
                   file_name, differences);
      return std::nullopt;
    }

    for (size_t grid_index = 0; grid_index < grid_size; ++grid_index) {
      const size_t row = grid_index_to_row[grid_index];
      if (row == SIZE_MAX)
        continue;
      const float value = sst[grid_index];
      dataset.matrix[row * num_days + day] =
          (value == fill_value) ? std::numeric_limits<float>::quiet_NaN()
                                : value;
    }
  }

  if (verbose)
    std::println("Read {} file(s) into a {} x {} matrix.",
                 time_axis.files.size(), num_rows, num_days);

  return dataset;
}

bool summarizeDataset(const SstDataset &dataset, bool verbose) {
  const size_t num_days = dataset.time.dayOffsets.size();
  const size_t num_rows = num_days ? dataset.matrix.size() / num_days : 0;

  size_t nan_count = 0;
  double sum = 0.0;
  bool has_value = false;
  float minV = 0.0f;
  float maxV = 0.0f;

  for (float v : dataset.matrix) {
    if (std::isnan(v)) {
      ++nan_count;
      continue;
    }
    sum += v;
    if (!has_value) {
      minV = v;
      maxV = v;
      has_value = true;
    } else {
      minV = std::min(minV, v);
      maxV = std::max(maxV, v);
    }
  }

  const size_t valid = dataset.matrix.size() - nan_count;
  if (verbose) {
    std::println();
    std::println("Rows (ocean pixels):    {}\n"
                 "Columns (time steps):   {}\n"
                 "NaN cells:              {}\n"
                 "Min / Max / Mean:       {} / {} / {}",
                 num_rows, num_days, nan_count,
                 has_value ? minV : 0.0f, has_value ? maxV : 0.0f,
                 (valid ? sum / valid : 0.0));
    std::println();
  }
  return nan_count == 0;
}
