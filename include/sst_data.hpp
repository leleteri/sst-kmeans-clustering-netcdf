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

std::optional<SstDataset> buildSstDataSet(const GridInfo &grid,
                                          const TimeAxis &time_axis);

#endif
