export module cluster.kmeans;

import std;

export class KMeans {
public:
  explicit KMeans(std::size_t k, unsigned seed = 42,
                  std::size_t max_iterations = 100)
      : m_k(k), m_seed(seed), m_max_iterations(max_iterations) {
    if (k == 0)
      throw std::invalid_argument("k must be greater than 0.");
    if (max_iterations == 0)
      throw std::invalid_argument("max_iterations must be greater than 0.");
  }

  void fit(std::span<const float> data, std::size_t num_rows,
           std::size_t num_cols) {
    if (m_k > num_rows) {
      throw std::invalid_argument(std::format(
          "k ({}) is larger than the number of rows ({}).", m_k, num_rows));
    }
    if (data.size() != num_rows * num_cols) {
      throw std::invalid_argument(
          "data size does not match num_rows * num_cols.");
    }
    if (!std::ranges::all_of(data, [](float v) { return std::isfinite(v); })) {
      throw std::invalid_argument(
          "data contains a non-finite value; fill it before clustering.");
    }

    m_num_cols = num_cols;
    m_labels.assign(num_rows, -1);
    m_converged = false;
    m_iterations = 0;

    std::mt19937 rng(m_seed);
    initializeCenters(data, num_rows, rng);

    while (m_iterations < m_max_iterations) {
      ++m_iterations;
      if (!assignLabels(data, num_rows)) {
        m_converged = true; // no row changed cluster
        break;
      }
      updateCenters(data, num_rows, rng);
    }
  }

  std::size_t k() const noexcept { return m_k; }
  std::size_t numCols() const noexcept { return m_num_cols; }
  // Cluster (0..k-1) of every row.
  const std::vector<int> &labels() const noexcept { return m_labels; }
  // Row-major [cluster][column].
  const std::vector<float> &centers() const noexcept { return m_centers; }
  bool converged() const noexcept { return m_converged; }
  std::size_t iterations() const noexcept { return m_iterations; }

  // Number of rows in every cluster.
  std::vector<std::size_t> clusterSizes() const {
    std::vector<std::size_t> sizes(m_k, 0);
    for (const int label : m_labels)
      ++sizes[static_cast<std::size_t>(label)];
    return sizes;
  }

private:
  std::size_t m_k;
  unsigned m_seed;
  std::size_t m_max_iterations;

  std::size_t m_num_cols = 0;
  std::vector<int> m_labels;
  std::vector<float> m_centers;
  bool m_converged = false;
  std::size_t m_iterations = 0;

  static double squaredDistance(const float *a, const float *b,
                                std::size_t num_cols) {
    double sum = 0.0;
    for (std::size_t j = 0; j < num_cols; ++j) {
      const double diff = static_cast<double>(a[j]) - static_cast<double>(b[j]);
      sum += diff * diff;
    }
    return sum;
  }

  void initializeCenters(std::span<const float> data, std::size_t num_rows,
                         std::mt19937 &rng) {
    std::vector<std::size_t> order(num_rows);
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::shuffle(order.begin(), order.end(), rng);

    m_centers.resize(m_k * m_num_cols);
    for (std::size_t c = 0; c < m_k; ++c)
      copyRow(data, order[c], &m_centers[c * m_num_cols]);
  }

  void copyRow(std::span<const float> data, std::size_t row,
               float *dest) const {
    std::copy_n(&data[row * m_num_cols], m_num_cols, dest);
  }

  bool assignLabels(std::span<const float> data, std::size_t num_rows) {
    bool changed = false;
    for (std::size_t row = 0; row < num_rows; ++row) {
      const float *point = &data[row * m_num_cols];

      double best_distance = std::numeric_limits<double>::max();
      int best_cluster = -1;
      for (std::size_t c = 0; c < m_k; ++c) {
        const double distance =
            squaredDistance(point, &m_centers[c * m_num_cols], m_num_cols);
        if (distance < best_distance) {
          best_distance = distance;
          best_cluster = static_cast<int>(c);
        }
      }

      if (best_cluster < 0) {
        throw std::invalid_argument(
            "a row could not be assigned to any cluster.");
      }
      if (m_labels[row] != best_cluster) {
        m_labels[row] = best_cluster;
        changed = true;
      }
    }
    return changed;
  }

  void updateCenters(std::span<const float> data, std::size_t num_rows,
                     std::mt19937 &rng) {
    std::vector<double> sums(m_k * m_num_cols, 0.0);
    std::vector<std::size_t> counts(m_k, 0);

    for (std::size_t row = 0; row < num_rows; ++row) {
      const auto c = static_cast<std::size_t>(m_labels[row]);
      ++counts[c];
      for (std::size_t j = 0; j < m_num_cols; ++j)
        sums[c * m_num_cols + j] += data[row * m_num_cols + j];
    }

    std::uniform_int_distribution<std::size_t> any_row(0, num_rows - 1);
    for (std::size_t c = 0; c < m_k; ++c) {
      if (counts[c] == 0) {
        copyRow(data, any_row(rng), &m_centers[c * m_num_cols]);
        continue;
      }
      for (std::size_t j = 0; j < m_num_cols; ++j) {
        m_centers[c * m_num_cols + j] = static_cast<float>(
            sums[c * m_num_cols + j] / static_cast<double>(counts[c]));
      }
    }
  }
};
