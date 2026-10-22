#ifndef SST_DATA_HPP
#define SST_DATA_HPP

#include "filename_parse.hpp"
#include "grid_data.hpp"
#include <optional>
#include <vector>

struct SstDataset {
  TimeAxis time;
  GridInfo grid;
  std::vector<float> matrix;
};

// Reads every file of the time axis, checks that the ocean mask is identical
// to the baseline mask and stores the values in a row-major
// (ocean pixel x time step) matrix. Fill values are stored as NaN.
std::optional<SstDataset> buildSstDataSet(const GridInfo &grid,
                                          const TimeAxis &time_axis,
                                          bool verbose = false);
bool summarizeDataset(const SstDataset &dataset, bool verbose);

#endif
