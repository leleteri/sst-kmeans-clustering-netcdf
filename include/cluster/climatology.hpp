#ifndef CLIMATOLOGY_HPP
#define CLIMATOLOGY_HPP

#include "cluster_data.hpp"
#include <cstddef>
#include <vector>

struct Climatology {
  std::vector<float> matrix;
  size_t num_rows = 0;
  static constexpr size_t num_months = 12;
};

Climatology buildClimatology(const ClusterInput &input);

#endif
