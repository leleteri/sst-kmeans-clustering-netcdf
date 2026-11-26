module;
#include <netcdf>

export module cluster.input;

import std;
import sst_schema;
import nc_utils;

namespace fs = std::filesystem;

export class ClusterInput {
public:
  explicit ClusterInput(const fs::path &path) {
    const NcReader reader(path);

    m_num_pixels = reader.dim(schema::PixelDim).getSize();
    m_num_days = reader.dim(schema::TimeDim).getSize();

    m_latitude = reader.readAxis<double>(schema::LatitudeVar);
    m_longitude = reader.readAxis<double>(schema::LongitudeVar);
    if (m_latitude.size() != m_num_pixels ||
        m_longitude.size() != m_num_pixels) {
      throw std::runtime_error(
          std::format("{}: latitude/longitude size does not match '{}'.",
                      reader.name(), schema::PixelDim));
    }

    m_days = reader.readAxis<int>(schema::TimeVar);
    if (m_days.size() != m_num_days) {
      throw std::runtime_error(
          std::format("{}: '{}' size does not match the '{}' dimension.",
                      reader.name(), schema::TimeVar, schema::TimeDim));
    }
    warnIfTimeIsNotDays(reader);

    readSst(reader);
  }

  const std::vector<double> &latitude() const noexcept { return m_latitude; }
  const std::vector<double> &longitude() const noexcept { return m_longitude; }
  const std::vector<int> &days() const noexcept { return m_days; }
  const std::vector<float> &sst() const noexcept { return m_sst; }
  std::size_t numPixels() const noexcept { return m_num_pixels; }
  std::size_t numDays() const noexcept { return m_num_days; }

private:
  std::vector<double> m_latitude;
  std::vector<double> m_longitude;
  std::vector<int> m_days;
  std::vector<float> m_sst;
  std::size_t m_num_pixels = 0;
  std::size_t m_num_days = 0;

  static void warnIfTimeIsNotDays(const NcReader &reader) {
    const netCDF::NcVar time = reader.var(schema::TimeVar);
    const netCDF::NcVarAtt units_att = reader.guard(
        [&] { return time.getAtt(std::string(schema::UnitsAtt)); });
    if (units_att.isNull())
      return;

    std::string units;
    reader.guard([&] { units_att.getValues(units); });
    if (!units.starts_with("days since")) {
      std::println(std::cerr,
                   "warning: {}: time units are '{}', but the climatology "
                   "expects days.",
                   reader.name(), units);
    }
  }

  void readSst(const NcReader &reader) {
    const netCDF::NcVar sst = reader.var(schema::SstVar);
    const std::vector<std::size_t> shape = reader.shape(sst, schema::SstVar, 2);
    if (shape[0] != m_num_pixels || shape[1] != m_num_days) {
      throw std::runtime_error(
          std::format("{}: '{}' does not have the expected shape ({} x {}).",
                      reader.name(), schema::SstVar, m_num_pixels, m_num_days));
    }

    m_sst.resize(m_num_pixels * m_num_days);
    reader.guard([&] { sst.getVar(m_sst.data()); });
  }
};
