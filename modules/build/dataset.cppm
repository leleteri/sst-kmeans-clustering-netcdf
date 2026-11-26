export module build.dataset;

import std;
import nc_utils;
import build.grid_info;
import build.sst_slice;
import build.time_axis;

namespace fs = std::filesystem;

// Matrix of SST values: one row per ocean pixel, one column per day, stored
// row-major as [pixel][day]. Every file is read exactly once and its ocean
// mask is compared with the baseline mask while it is read.
export class Dataset {
public:
  struct Summary {
    std::size_t pixels = 0;
    std::size_t days = 0;
    std::size_t missing = 0; // NaN cells
    float min = 0.0f;
    float max = 0.0f;
    double mean = 0.0;

    bool complete() const noexcept { return missing == 0; }

    void print() const {
      std::println("\n"
                   "Rows (ocean pixels):    {}\n"
                   "Columns (time steps):   {}\n"
                   "NaN cells:              {}\n"
                   "Min / Max / Mean:       {} / {} / {}\n",
                   pixels, days, missing, min, max, mean);
    }
  };

  Dataset() = default;
  Dataset(const GridInfo &grid, const TimeAxis &time_axis,
          bool verbose = false) {
    build(grid, time_axis, verbose);
  }

  // Row-major [pixel][day].
  const std::vector<float> &matrix() const noexcept { return m_matrix; }
  std::size_t numPixels() const noexcept { return m_num_pixels; }
  std::size_t numDays() const noexcept { return m_num_days; }

  explicit operator bool() const noexcept { return !m_matrix.empty(); }

  Summary summary() const {
    Summary result;
    result.pixels = m_num_pixels;
    result.days = m_num_days;

    double sum = 0.0;
    std::size_t valid = 0;
    float min_v = std::numeric_limits<float>::max();
    float max_v = std::numeric_limits<float>::lowest();
    for (const float v : m_matrix) {
      if (std::isnan(v))
        continue;
      sum += v;
      ++valid;
      min_v = std::min(min_v, v);
      max_v = std::max(max_v, v);
    }

    result.missing = m_matrix.size() - valid;
    if (valid > 0) {
      result.min = min_v;
      result.max = max_v;
      result.mean = sum / static_cast<double>(valid);
    }
    return result;
  }

private:
  std::vector<float> m_matrix;
  std::size_t m_num_pixels = 0;
  std::size_t m_num_days = 0;

  void build(const GridInfo &grid, const TimeAxis &time_axis, bool verbose) {
    if (!grid || !time_axis)
      throw std::invalid_argument("grid or time axis is empty.");

    const std::vector<bool> &mask = grid.oceanMask();
    const std::size_t num_lats = grid.latitude().size();
    const std::size_t num_lons = grid.longitude().size();
    const std::size_t num_pixels = grid.numOceanCells();
    const std::size_t num_days = time_axis.files().size();

    if (num_pixels == 0)
      throw std::runtime_error("ocean mask contains no ocean pixels.");

    m_num_pixels = num_pixels;
    m_num_days = num_days;
    m_matrix.assign(num_pixels * num_days,
                    std::numeric_limits<float>::quiet_NaN());

    for (std::size_t day = 0; day < num_days; ++day) {
      const fs::path &path = time_axis.files()[day];
      const NcReader reader(path);
      const SstSlice slice(reader, num_lats, num_lons);

      // Ocean cells are visited in grid order, which is the row order, so a
      // running counter replaces a cell -> row lookup table.
      std::size_t row = 0;
      std::size_t differences = 0;
      for (std::size_t cell = 0; cell < mask.size(); ++cell) {
        // A cell is ocean when it is not fill; it must agree with the baseline.
        if (slice.isFill(cell) == mask[cell])
          ++differences;
        if (!mask[cell])
          continue;
        m_matrix[row * num_days + day] = slice.values()[cell];
        ++row;
      }

      if (differences > 0) {
        throw std::runtime_error(
            std::format("{}: {} pixel(s) differ from the baseline ocean mask.",
                        reader.name(), differences));
      }
    }

    if (verbose) {
      std::println("read {} file(s) into a {} x {} matrix.", num_days,
                   num_pixels, num_days);
    }
  }
};
