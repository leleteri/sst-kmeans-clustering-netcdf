module;
#include <netcdf>

export module build.grid_info;

import std;
import sst_schema;
import nc_utils;
import build.sst_slice;

namespace fs = std::filesystem;

export class GridAxes {
public:
  GridAxes() = default;

  static GridAxes read(const NcReader &reader) {
    GridAxes axes;
    axes.m_latitude = reader.readAxis<double>(schema::LatitudeVar);
    axes.m_longitude = reader.readAxis<double>(schema::LongitudeVar);
    return axes;
  }

  const std::vector<double> &latitude() const noexcept { return m_latitude; }
  const std::vector<double> &longitude() const noexcept { return m_longitude; }
  std::size_t numCells() const noexcept {
    return m_latitude.size() * m_longitude.size();
  }

  bool sameLatitude(const GridAxes &other, double tolerance) const {
    return sameValues(m_latitude, other.m_latitude, tolerance);
  }
  bool sameLongitude(const GridAxes &other, double tolerance) const {
    return sameValues(m_longitude, other.m_longitude, tolerance);
  }

private:
  std::vector<double> m_latitude;
  std::vector<double> m_longitude;

  static bool sameValues(const std::vector<double> &a,
                         const std::vector<double> &b, double tolerance) {
    if (a.size() != b.size())
      return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
      if (std::isnan(a[i]) || std::isnan(b[i]))
        return false;
      if (std::abs(a[i] - b[i]) > tolerance)
        return false;
    }
    return true;
  }
};

export class GridInfo {
public:
  GridInfo() = default;
  explicit GridInfo(const fs::path &path) {
    const NcReader reader(path);
    m_axes = GridAxes::read(reader);

    const SstSlice slice(reader, m_axes.latitude().size(),
                         m_axes.longitude().size());
    m_fill_value = slice.fillValue();
    m_ocean_mask.resize(slice.size());
    for (std::size_t i = 0; i < slice.size(); ++i)
      m_ocean_mask[i] = !slice.isFill(i);
  }

  const GridAxes &axes() const noexcept { return m_axes; }
  const std::vector<double> &latitude() const noexcept {
    return m_axes.latitude();
  }
  const std::vector<double> &longitude() const noexcept {
    return m_axes.longitude();
  }
  const std::vector<bool> &oceanMask() const noexcept { return m_ocean_mask; }
  float fillValue() const noexcept { return m_fill_value; }

  std::size_t numOceanCells() const {
    return static_cast<std::size_t>(std::ranges::count(m_ocean_mask, true));
  }

  explicit operator bool() const noexcept {
    return m_axes.numCells() > 0 && m_ocean_mask.size() == m_axes.numCells();
  }

private:
  GridAxes m_axes;
  std::vector<bool> m_ocean_mask;
  float m_fill_value = 0.0f;
};
