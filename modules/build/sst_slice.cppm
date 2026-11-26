module;
#include <netcdf>

export module build.sst_slice;

import std;
import sst_schema;
import nc_utils;

export class SstSlice {
public:
  SstSlice(const NcReader &reader, std::size_t num_lats, std::size_t num_lons) {
    const netCDF::NcVar sst = reader.var(schema::SstVar);

    const std::vector<std::size_t> shape = reader.shape(sst, schema::SstVar, 3);
    if (shape[0] < 1 || shape[1] != num_lats || shape[2] != num_lons) {
      throw std::runtime_error(
          std::format("{}: '{}' does not have the expected shape.",
                      reader.name(), schema::SstVar));
    }

    const netCDF::NcVarAtt fill =
        reader.att(sst, schema::SstVar, schema::FillValueAtt);
    reader.guard([&] { fill.getValues(&m_fill_value); });

    const std::vector<std::size_t> start = {0, 0, 0};
    const std::vector<std::size_t> count = {1, num_lats, num_lons};
    m_values.resize(num_lats * num_lons);
    reader.guard([&] { sst.getVar(start, count, m_values.data()); });
  }

  const std::vector<float> &values() const noexcept { return m_values; }
  std::size_t size() const noexcept { return m_values.size(); }
  float fillValue() const noexcept { return m_fill_value; }

  bool isFill(std::size_t index) const noexcept {
    const float value = m_values[index];
    return value == m_fill_value ||
           (std::isnan(value) && std::isnan(m_fill_value));
  }

private:
  std::vector<float> m_values;
  float m_fill_value = 0.0f;
};
