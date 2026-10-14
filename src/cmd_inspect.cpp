#include "arguments.hpp"
#include "commands.hpp"
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <netcdf>

namespace fs = std::filesystem;

namespace {
void printAttribute(const std::string &name, const netCDF::NcVarAtt &att) {
  std::cout << "      " << name << " = ";

  const netCDF::NcType type = att.getType();
  const size_t len = att.getAttLength();

  try {
    if (type == netCDF::ncChar) {
      std::string value;
      att.getValues(value);
      std::cout << "\"" << value << "\"";
    } else if (type == netCDF::ncFloat && len == 1) {
      float value;
      att.getValues(&value);
      std::cout << value;
    } else if (type == netCDF::ncDouble && len == 1) {
      double value;
      att.getValues(&value);
      std::cout << value;
    } else if (type == netCDF::ncInt && len == 1) {
      int value;
      att.getValues(&value);
      std::cout << value;
    } else if (type == netCDF::ncShort && len == 1) {
      short value;
      att.getValues(&value);
      std::cout << value;
    } else {
      std::cout << "<" << type.getName() << ", " << len << " value(s)>";
    }
  } catch (const netCDF::exceptions::NcException &exception) {
    std::cout << "<unreadable: " << exception.what() << ">";
  }

  std::cout << "\n";
}

bool inspectFile(const fs::path &path) {
  try {
    netCDF::NcFile file(path.string(), netCDF::NcFile::read);
    std::cout << "File: " << path << "\n";

    std::cout << "  Dimensions:\n";
    for (const auto &[name, dim] : file.getDims())
      std::cout << "    " << name << " = " << dim.getSize() << "\n";

    std::cout << "  Variables:\n";
    for (const auto &[name, var] : file.getVars()) {
      std::cout << "    " << name << " (" << var.getType().getName() << ")";

      auto dims = var.getDims();
      if (!dims.empty()) {
        std::cout << " [";
        for (size_t i = 0; i < dims.size(); ++i) {
          std::cout << dims[i].getName();
          if (i + 1 < dims.size())
            std::cout << ", ";
        }
        std::cout << "]";
      }
      std::cout << "\n";

      for (const auto &[attName, att] : var.getAtts())
        printAttribute(attName, att);
    }

    std::cout << "\n";
    return true;

  } catch (const netCDF::exceptions::NcException &exception) {
    std::cerr << "NetCDF error [" << path << "]: " << exception.what() << "\n";
    return false;
  }
}

} // namespace

int runInspect(const std::vector<std::string> &args) {
  InspectOptions opts;
  try {
    opts = parseInspectArgs(args);
  } catch (const std::exception &exception) {
    std::cerr << "Arguments error: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }

  bool allOk = true;
  for (const auto &input : opts.inputs) {
    if (!inspectFile(input))
      allOk = false;
  }

  return allOk ? EXIT_SUCCESS : EXIT_FAILURE;
}
