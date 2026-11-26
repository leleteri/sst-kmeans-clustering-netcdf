module;
#include <netcdf>

export module inspect.inspector;

import std;
import nc_utils;

namespace fs = std::filesystem;

export class NcInspector {
public:
  explicit NcInspector(bool verbose = false) : m_verbose(verbose) {}

  void inspect(const fs::path &path) const {
    const NcReader reader(path);
    const netCDF::NcFile &file = reader.file();

    reader.guard([&] {
      std::println("File: {}", path.string());

      std::println("  Dimensions:");
      for (const auto &[name, dim] : file.getDims())
        std::println("    {} = {}", name, dim.getSize());

      std::println("  Variables:");
      for (const auto &[name, var] : file.getVars())
        printVariable(name, var);

      if (m_verbose)
        printGlobalAttributes(file);
      std::println("");
    });
  }

private:
  bool m_verbose;

  static void printVariable(const std::string &name, const netCDF::NcVar &var) {
    std::print("    {} ({})", name, var.getType().getName());

    const std::vector<netCDF::NcDim> dims = var.getDims();
    if (!dims.empty()) {
      std::print(" [");
      for (std::size_t i = 0; i < dims.size(); ++i)
        std::print("{}{}", i > 0 ? ", " : "", dims[i].getName());
      std::print("]");
    }
    std::println("");

    for (const auto &[att_name, att] : var.getAtts())
      printAttribute(att_name, att);
  }

  static void printGlobalAttributes(const netCDF::NcFile &file) {
    const auto attributes = file.getAtts();
    if (attributes.empty())
      return;

    std::println("  Global attributes:");
    for (const auto &[name, att] : attributes)
      printAttribute(name, att);
  }

  template <typename T> static void printScalar(const netCDF::NcAtt &att) {
    T value{};
    att.getValues(&value);
    std::print("{}", value);
  }

  static void printAttribute(const std::string &name,
                             const netCDF::NcAtt &att) {
    std::print("      {} = ", name);

    const netCDF::NcType type = att.getType();
    const std::size_t length = att.getAttLength();

    try {
      if (type == netCDF::ncChar) {
        std::string value;
        att.getValues(value);
        std::print("\"{}\"", value);
      } else if (length == 1 && type == netCDF::ncFloat) {
        printScalar<float>(att);
      } else if (length == 1 && type == netCDF::ncDouble) {
        printScalar<double>(att);
      } else if (length == 1 && type == netCDF::ncInt) {
        printScalar<int>(att);
      } else if (length == 1 && type == netCDF::ncShort) {
        printScalar<short>(att);
      } else {
        std::print("<{}, {} value(s)>", type.getName(), length);
      }
    } catch (const netCDF::exceptions::NcException &e) {
      std::print("<unreadable: {}>", e.what());
    }
    std::println("");
  }
};
