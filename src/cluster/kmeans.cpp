#include "cluster/kmeans.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

double squaredDistance(const float *a, const float *b, size_t num_cols) {
  double sum = 0.0;
  for (size_t j = 0; j < num_cols; ++j) {
    const double diff = static_cast<double>(a[j]) - static_cast<double>(b[j]);
    sum += diff * diff;
  }
  return sum;
}

std::vector<float> initializeCenters(const std::vector<float> &data,
                                     size_t num_rows, size_t num_cols, size_t k,
                                     unsigned seed) {
  std::vector<size_t> rowIndex(num_rows);
  for (size_t i = 0; i < num_rows; ++i)
    rowIndex[i] = i;

  std::mt19937 rng(seed);
  std::shuffle(rowIndex.begin(), rowIndex.end(), rng);

  std::vector<float> centers(k * num_cols);
  for (size_t c = 0; c < k; ++c) {
    size_t row = rowIndex[c];
    std::copy_n(&data[row * num_cols], num_cols, &centers[c * num_cols]);
  }
  return centers;
}

} // namespace

KMeansResult runKMeans(const std::vector<float> &data, size_t num_rows,
                       size_t num_cols, size_t k, unsigned seed,
                       size_t max_iterations) {
  if (k == 0 || k > num_rows)
    throw std::invalid_argument("k must be between 1 and num_rows");
  if (data.size() != num_rows * num_cols)
    throw std::invalid_argument("data size does not match num_rows * num_cols");

  for (float value : data) {
    if (!std::isfinite(value))
      throw std::invalid_argument(
          "data contains a non-finite value; fill it before clustering");
  }

  KMeansResult result;
  result.labels.assign(num_rows, -1);
  result.centers = initializeCenters(data, num_rows, num_cols, k, seed);

  std::mt19937 rng(seed);

  for (size_t iter = 0; iter < max_iterations; ++iter) {
    bool anyChanged = false;

    for (size_t row = 0; row < num_rows; ++row) {
      const float *point = &data[row * num_cols];

      double best_dist = std::numeric_limits<double>::max();
      int best_cluster = -1;
      for (size_t c = 0; c < k; ++c) {
        const double dist =
            squaredDistance(point, &result.centers[c * num_cols], num_cols);
        if (dist < best_dist) {
          best_dist = dist;
          best_cluster = static_cast<int>(c);
        }
      }

      // Every distance was NaN, so the row could not be assigned. Leaving the
      // label at -1 would corrupt the cluster statistics below.
      if (best_cluster < 0)
        throw std::invalid_argument(
            "a row could not be assigned to any cluster");

      if (result.labels[row] != best_cluster) {
        result.labels[row] = best_cluster;
        anyChanged = true;
      }
    }

    if (!anyChanged)
      break; // converged: no row changed cluster this pass

    std::vector<double> sums(k * num_cols, 0.0);
    std::vector<size_t> counts(k, 0);

    for (size_t row = 0; row < num_rows; ++row) {
      const int c = result.labels[row];
      if (c < 0 || static_cast<size_t>(c) >= k)
        throw std::logic_error("cluster label out of range");
      counts[c] += 1;
      const float *point = &data[row * num_cols];
      double *sumRow = &sums[c * num_cols];
      for (size_t j = 0; j < num_cols; ++j)
        sumRow[j] += point[j];
    }

    for (size_t c = 0; c < k; ++c) {
      if (counts[c] == 0) {
        // Re-seed an empty cluster with a random existing row.
        std::uniform_int_distribution<size_t> dist(0, num_rows - 1);
        const size_t row = dist(rng);
        std::copy_n(&data[row * num_cols], num_cols,
                    &result.centers[c * num_cols]);
        continue;
      }
      for (size_t j = 0; j < num_cols; ++j)
        result.centers[c * num_cols + j] =
            static_cast<float>(sums[c * num_cols + j] / counts[c]);
    }
  }

  return result;
}
