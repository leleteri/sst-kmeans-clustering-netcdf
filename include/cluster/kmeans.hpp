#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <cstddef>
#include <vector>

struct KMeansResult {
  std::vector<int> labels;
  std::vector<float> centers;
};

// Clusters the rows of data (row-major, num_cols values per row).
// Throws std::invalid_argument when the arguments are inconsistent or when the
// data contains a non-finite value.
KMeansResult runKMeans(const std::vector<float> &data, size_t num_rows,
                       size_t num_cols, size_t k, unsigned seed,
                       size_t max_iterations = 100);

#endif
