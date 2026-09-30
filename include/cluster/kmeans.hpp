#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <vector>

struct KMeansResult {
  std::vector<int> labels;
  std::vector<float> centers;
};

KMeansResult runKMeans(const std::vector<float> &data, size_t num_rows,
                       size_t num_cols, size_t k, unsigned seed,
                       size_t max_iterations = 100);

#endif
