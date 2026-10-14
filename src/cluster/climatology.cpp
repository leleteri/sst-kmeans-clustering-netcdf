#include "cluster/climatology.hpp"
#include "cluster/cluster_data.hpp"

#include <chrono>
#include <cmath>
#include <limits>

namespace df = std::chrono;

static unsigned monthOf(long days_since_epoch) {
  df::sys_days sys_days{df::days{days_since_epoch}};
  df::year_month_day dateYMD{sys_days};
  return static_cast<unsigned>(dateYMD.month());
}

Climatology buildClimatology(const ClusterInput &input) {
  std::vector<unsigned> column_month(input.num_cols);
  for (size_t col = 0; col < input.num_cols; ++col) {
    column_month[col] = monthOf(input.days[col]);
  }

  const size_t num_rows = input.num_rows;
  const size_t num_cols = input.num_cols;
  const size_t num_months = Climatology::num_months;

  std::vector<double> sums(num_rows * num_months, 0.0);
  std::vector<unsigned> counts(num_rows * num_months, 0);

  for (size_t row = 0; row < num_rows; ++row) {
    const size_t row_offset = row * num_cols;
    for (size_t col = 0; col < num_cols; ++col) {
      float value = input.sst[row_offset + col];
      if (std::isnan(value))
        continue;

      unsigned month = column_month[col];
      size_t bin = row * num_months + (month - 1);
      sums[bin] += value;
      counts[bin] += 1;
    }
  }

  Climatology climatology;
  climatology.num_rows = num_rows;
  climatology.matrix.resize(num_rows * num_months);

  for (size_t bin = 0; bin < climatology.matrix.size(); ++bin) {
    climatology.matrix[bin] = counts[bin] > 0
                                  ? static_cast<float>(sums[bin] / counts[bin])
                                  : std::numeric_limits<float>::quiet_NaN();
  }

  return climatology;
}
