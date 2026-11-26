export module cluster.climatology;

import std;
import cluster.input;

namespace chrono = std::chrono;

export class Climatology {
public:
  static constexpr std::size_t NumMonths = 12;

  explicit Climatology(const ClusterInput &input)
      : m_num_pixels(input.numPixels()) {
    const std::size_t num_days = input.numDays();

    std::vector<unsigned> month_of_day(num_days);
    for (std::size_t day = 0; day < num_days; ++day)
      month_of_day[day] = monthOf(input.days()[day]);

    std::vector<double> sums(m_num_pixels * NumMonths, 0.0);
    std::vector<unsigned> counts(m_num_pixels * NumMonths, 0);

    for (std::size_t pixel = 0; pixel < m_num_pixels; ++pixel) {
      const std::size_t row = pixel * num_days;
      for (std::size_t day = 0; day < num_days; ++day) {
        const float value = input.sst()[row + day];
        if (std::isnan(value))
          continue;
        const std::size_t bin = pixel * NumMonths + (month_of_day[day] - 1);
        sums[bin] += value;
        counts[bin] += 1;
      }
    }

    m_matrix.resize(sums.size());
    for (std::size_t bin = 0; bin < m_matrix.size(); ++bin) {
      m_matrix[bin] = counts[bin] > 0
                          ? static_cast<float>(sums[bin] / counts[bin])
                          : std::numeric_limits<float>::quiet_NaN();
    }
  }

  const std::vector<float> &matrix() const noexcept { return m_matrix; }
  std::size_t numPixels() const noexcept { return m_num_pixels; }

  std::size_t missingCells() const {
    return static_cast<std::size_t>(
        std::ranges::count_if(m_matrix, [](float v) { return std::isnan(v); }));
  }

private:
  std::vector<float> m_matrix;
  std::size_t m_num_pixels = 0;

  static unsigned monthOf(int days_since_epoch) {
    const chrono::year_month_day date{
        chrono::sys_days{chrono::days{days_since_epoch}}};
    return static_cast<unsigned>(date.month());
  }
};
