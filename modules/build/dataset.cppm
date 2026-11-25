module;
#include <netcdf>

export module dataset;

import std;
import grid_info;
import time_axis;

namespace fs = std::filesystem;

export class Dataset {
public:
  Dataset() = default;
  explicit Dataset(const GridInfo &grid, const TimeAxis &time_axis,
                   bool verbose = false) {
    buildDataset(grid, time_axis, verbose);
  }

  const std::vector<float> &matrix() const { return m_matrix; }
  std::size_t numRows() const noexcept { return m_num_rows; }
  std::size_t numDays() const noexcept { return m_num_days; }

  explicit operator bool() const { return !m_matrix.empty(); }

  bool summarize(bool verbose) const {
    if (m_matrix.empty())
      return false;

    std::size_t nan_count = 0;
    double sum = 0.0;
    bool has_value = false;
    float min_v = 0.0f;
    float max_v = 0.0f;

    for (float v : m_matrix) {
      if (std::isnan(v)) {
        ++nan_count;
        continue;
      }
      sum += v;
      if (!has_value) {
        min_v = v;
        max_v = v;
        has_value = true;
      } else {
        min_v = std::min(min_v, v);
        max_v = std::max(max_v, v);
      }
    }

    const std::size_t valid = m_matrix.size() - nan_count;
    if (verbose) {
      std::println();
      std::println("Rows (ocean pixels):    {}\n"
                   "Columns (time steps):   {}\n"
                   "NaN cells:              {}\n"
                   "Min / Max / Mean:       {} / {} / {}",
                   m_num_rows, m_num_days, nan_count, min_v, max_v,
                   valid ? sum / valid : 0.0);
      std::println();
    }
    return nan_count == 0;
  }

private:
  struct DayData {
    std::vector<float> sst;
    float fill_value = 0.0f;
  };

  static constexpr std::string_view SstVarName = "analysed_sst";
  static constexpr std::string_view FillAttName = "_FillValue";

  std::vector<float> m_matrix;
  std::size_t m_num_rows = 0;
  std::size_t m_num_days = 0;

  void buildDataset(const GridInfo &grid, const TimeAxis &time_axis,
                    bool verbose) {
    if (!grid || !time_axis)
      throw std::invalid_argument("grid or time axis is empty.");

    const std::vector<bool> &ocean_mask = grid.oceanMask();
    const std::size_t num_lats = grid.latitude().size();
    const std::size_t num_lons = grid.longitude().size();
    const std::size_t grid_size = num_lats * num_lons;
    const std::size_t num_days = time_axis.dayOffsets().size();

    if (grid_size != ocean_mask.size()) {
      throw std::runtime_error(
          "ocean mask size does not match the coordinate grid.");
    }
    if (time_axis.files().size() != num_days) {
      throw std::runtime_error(
          "time axis has a different number of files and dates.");
    }

    const std::size_t num_rows = static_cast<std::size_t>(
        std::count(ocean_mask.begin(), ocean_mask.end(), true));
    if (num_rows == 0)
      throw std::runtime_error("ocean mask contains no ocean pixels.");

    m_matrix.assign(num_rows * num_days,
                    std::numeric_limits<float>::quiet_NaN());
    m_num_rows = num_rows;
    m_num_days = num_days;

    std::vector<std::size_t> grid_index_to_row(grid_size, SIZE_MAX);
    std::size_t row_counter = 0;
    for (std::size_t i = 0; i < grid_size; ++i) {
      if (ocean_mask[i])
        grid_index_to_row[i] = row_counter++;
    }

    for (std::size_t day = 0; day < num_days; ++day) {
      const fs::path &path = time_axis.files()[day];
      const DayData data = readDay(path, num_lats, num_lons);

      std::size_t differences = 0;
      for (std::size_t i = 0; i < grid_size; ++i) {
        if ((data.sst[i] != data.fill_value) != ocean_mask[i])
          ++differences;
      }
      if (differences > 0) {
        throw std::runtime_error(
            std::format("{}: {} pixel(s) differ from the baseline ocean mask.",
                        path.filename().string(), differences));
      }

      for (std::size_t grid_index = 0; grid_index < grid_size; ++grid_index) {
        const std::size_t row = grid_index_to_row[grid_index];
        if (row == SIZE_MAX)
          continue;
        const float value = data.sst[grid_index];
        m_matrix[row * num_days + day] =
            (value == data.fill_value) ? std::numeric_limits<float>::quiet_NaN()
                                       : value;
      }
    }

    if (verbose) {
      std::println("Read {} file(s) into a {} x {} matrix.", num_days, num_rows,
                   num_days);
    }
  }

  // Reads the first time step of 'analysed_sst' and its _FillValue.
  static DayData readDay(const fs::path &path, std::size_t num_lats,
                         std::size_t num_lons) {
    const std::string file_name = path.filename().string();
    DayData data;

    try {
      const netCDF::NcFile file(path.string(), netCDF::NcFile::read);

      netCDF::NcVar sst_var = file.getVar(SstVarName.data());
      if (sst_var.isNull()) {
        throw std::runtime_error(std::format(
            "{}: required variable '{}' not found.", file_name, SstVarName));
      }

      const std::vector<netCDF::NcDim> dims = sst_var.getDims();
      if (dims.size() != 3 || dims[0].getSize() < 1 ||
          dims[1].getSize() != num_lats || dims[2].getSize() != num_lons) {
        throw std::runtime_error(
            std::format("{}: '{}' does not have the expected shape.", file_name,
                        SstVarName));
      }

      netCDF::NcVarAtt fill_att = sst_var.getAtt(FillAttName.data());
      if (fill_att.isNull()) {
        throw std::runtime_error(std::format("{}: '{}' has no {}.", file_name,
                                             SstVarName, FillAttName));
      }
      fill_att.getValues(&data.fill_value);

      const std::vector<std::size_t> start = {0, 0, 0};
      const std::vector<std::size_t> count = {1, num_lats, num_lons};
      data.sst.resize(num_lats * num_lons);
      sst_var.getVar(start, count, data.sst.data());
    } catch (const netCDF::exceptions::NcException &e) {
      throw std::runtime_error(
          std::format("{}: netcdf: {}", file_name, e.what()));
    }

    return data;
  }
};
