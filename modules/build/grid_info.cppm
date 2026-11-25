module;
#include <netcdf>

export module grid_info;

import std;

namespace fs = std::filesystem;

export class GridInfo {
public:
  GridInfo() = default;
  explicit GridInfo(const fs::path &path) {
    const netCDF::NcFile file(path, netCDF::NcFile::read);
    auto file_name = path.string();
    readGridInfo(file, file_name);
  }

  const std::vector<double> &latitude() const { return m_latitude; }
  const std::vector<double> &longitude() const { return m_longitude; }
  const std::vector<bool> &oceanMask() const { return m_ocean_mask; }
  const float &fillValue() const noexcept { return m_fill_value; }
  // std::string_view crsWkt() const { return m_crs_wkt; }

  explicit operator bool() const {
    return (!m_latitude.empty() || !m_longitude.empty() ||
            !m_ocean_mask.empty());
  }

private:
  std::vector<double> m_latitude;
  std::vector<double> m_longitude;
  std::vector<bool> m_ocean_mask;
  float m_fill_value = 0.0f;
  // std::string m_crs_wkt;

  void readGridInfo(const netCDF::NcFile &file, std::string_view file_name) {
    constexpr std::string_view lat_var_name = "latitude";
    constexpr std::string_view lon_var_name = "longitude";
    constexpr std::string_view sst_var_name = "analysed_sst";
    // constexpr std::string_view crs_var_name = "crs";
    constexpr std::string_view fill_att_name = "_FillValue";
    // constexpr std::string_view wkt_att_name = "crs_wkt";

    try {
      netCDF::NcVar lat_var = file.getVar(lat_var_name.data());
      netCDF::NcVar lon_var = file.getVar(lon_var_name.data());
      netCDF::NcVar sst_var = file.getVar(sst_var_name.data());
      // netCDF::NcVar crs_var = file.getVar(crs_var_name.data());

      varNotNull(file_name, lat_var, lat_var_name);
      varNotNull(file_name, lon_var, lon_var_name);
      varNotNull(file_name, sst_var, sst_var_name);
      // varNotNull(file_name, crs_var, crs_var_name);

      std::vector<netCDF::NcDim> lat_dims = lat_var.getDims();
      std::vector<netCDF::NcDim> lon_dims = lon_var.getDims();
      std::vector<netCDF::NcDim> sst_dims = sst_var.getDims();

      dimsSizeMatch(file_name, lat_dims, 1, lat_var_name);
      dimsSizeMatch(file_name, lon_dims, 1, lon_var_name);
      dimsSizeMatch(file_name, sst_dims, 3, sst_var_name);

      std::size_t num_lat = lat_dims[0].getSize();
      std::size_t num_lon = lon_dims[0].getSize();

      m_latitude.resize(num_lat);
      m_longitude.resize(num_lon);
      lon_var.getVar(m_longitude.data());
      lat_var.getVar(m_latitude.data());

      netCDF::NcVarAtt fill_att = sst_var.getAtt(fill_att_name.data());
      // netCDF::NcVarAtt wkt_att = crs_var.getAtt(wkt_att_name.data());

      attNotNull(file_name, fill_att, sst_var_name, fill_att_name);
      // attNotNull(file_name, wkt_att, crs_var_name, wkt_att_name);

      fill_att.getValues(&m_fill_value);
      // wkt_att.getValues(&m_crs_wkt);

      if (sst_dims[0].getSize() < 1 || sst_dims[1].getSize() != num_lat ||
          sst_dims[2].getSize() != num_lon) {
        throw std::runtime_error(
            std::format("{}: '{}' does not have the expected shape.", file_name,
                        sst_var_name));
      }

      const std::vector<std::size_t> start = {0, 0, 0};
      const std::vector<std::size_t> count = {1, num_lat, num_lon};
      std::vector<float> values(count[0] * count[1] * count[2]);
      sst_var.getVar(start, count, values.data());

      m_ocean_mask.resize(values.size());
      for (std::size_t i = 0; i < values.size(); ++i) {
        m_ocean_mask[i] = values[i] != m_fill_value;
      }
    } catch (const netCDF::exceptions::NcException &e) {
      throw std::runtime_error(std::format("netcdf: {}", e.what()));
    }
  }

  static void varNotNull(std::string_view file_name, const netCDF::NcVar &var,
                         std::string_view var_name) {
    if (var.isNull()) {
      throw std::runtime_error(std::format(
          "{}: required variable '{}' not found.", file_name, var_name));
    }
  }

  static void attNotNull(std::string_view file_name,
                         const netCDF::NcVarAtt &att, std::string_view var_name,
                         std::string_view att_name) {

    if (att.isNull()) {
      throw std::runtime_error(
          std::format("{}: '{}' has no {}.", file_name, var_name, att_name));
    }
  }

  static void dimsSizeMatch(std::string_view file_name,
                            const std::vector<netCDF::NcDim> &dims,
                            const std::size_t &expected_size,
                            std::string_view var_name) {
    if (dims.size() != expected_size) {
      throw std::runtime_error(
          std::format("{}: expected dimension(s) for '{}' is {}, got {}.",
                      file_name, var_name, expected_size, dims.size()));
    }
  }
};
