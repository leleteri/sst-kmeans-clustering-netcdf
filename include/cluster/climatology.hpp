#ifndef CLIMATOLOGY_HPP
#define CLIMATOLOGY_HPP

#include "cluster/cluster_data.hpp"
#include <vector>

struct Climatology {
  std::vector<float> matrix;
  size_t num_rows;
  static constexpr size_t num_months = 12;

  float at(size_t row, unsigned month /* 1-12 */) const {
    return matrix[row * num_months + (month - 1)];
  }
};

Climatology buildClimatology(const ClusterInput &input);

#endif
