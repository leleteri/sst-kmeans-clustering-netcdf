module;
#include <netcdf>

export module nc_utils;

import std;

namespace fs = std::filesystem;

export template <typename Fn>
decltype(auto) ncGuard(std::string_view file_name, Fn &&fn) {
  try {
    return std::forward<Fn>(fn)();
  } catch (const netCDF::exceptions::NcException &e) {
    throw std::runtime_error(
        std::format("{}: netcdf: {}", file_name, e.what()));
  }
}

export class NcReader {
public:
  explicit NcReader(const fs::path &path) : m_name(path.filename().string()) {
    ncGuard(m_name, [&] { m_file.open(path.string(), netCDF::NcFile::read); });
  }

  NcReader(const NcReader &) = delete;
  NcReader &operator=(const NcReader &) = delete;

  const std::string &name() const noexcept { return m_name; }
  const netCDF::NcFile &file() const noexcept { return m_file; }

  template <typename Fn> decltype(auto) guard(Fn &&fn) const {
    return ncGuard(m_name, std::forward<Fn>(fn));
  }

  netCDF::NcVar var(std::string_view name) const {
    const netCDF::NcVar found =
        guard([&] { return m_file.getVar(std::string(name)); });
    if (found.isNull()) {
      throw std::runtime_error(
          std::format("{}: required variable '{}' not found.", m_name, name));
    }
    return found;
  }

  netCDF::NcDim dim(std::string_view name) const {
    const netCDF::NcDim found =
        guard([&] { return m_file.getDim(std::string(name)); });
    if (found.isNull()) {
      throw std::runtime_error(
          std::format("{}: required dimension '{}' not found.", m_name, name));
    }
    return found;
  }

  netCDF::NcVarAtt att(const netCDF::NcVar &var, std::string_view var_name,
                       std::string_view att_name) const {
    const netCDF::NcVarAtt found =
        guard([&] { return var.getAtt(std::string(att_name)); });
    if (found.isNull()) {
      throw std::runtime_error(
          std::format("{}: '{}' has no {}.", m_name, var_name, att_name));
    }
    return found;
  }

  // The variable's dimension sizes, after checking that it has `rank` of them.
  std::vector<std::size_t> shape(const netCDF::NcVar &var,
                                 std::string_view var_name,
                                 std::size_t rank) const {
    const std::vector<netCDF::NcDim> dims =
        guard([&] { return var.getDims(); });
    if (dims.size() != rank) {
      throw std::runtime_error(
          std::format("{}: '{}' must have {} dimension(s), got {}.", m_name,
                      var_name, rank, dims.size()));
    }
    std::vector<std::size_t> sizes;
    sizes.reserve(dims.size());
    for (const netCDF::NcDim &dim : dims)
      sizes.push_back(dim.getSize());
    return sizes;
  }

  template <typename T>
  std::vector<T> readAxis(std::string_view var_name) const {
    const netCDF::NcVar variable = var(var_name);
    std::vector<T> values(shape(variable, var_name, 1)[0]);
    guard([&] { variable.getVar(values.data()); });
    return values;
  }

private:
  std::string m_name;
  netCDF::NcFile m_file;
};
