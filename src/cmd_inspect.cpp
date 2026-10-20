#include "arguments.hpp"
#include "commands.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <netcdf>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
void printAttribute(const std::string &name, const netCDF::NcVarAtt &att) {
  std::print("      {} = ", name);

  const netCDF::NcType type = att.getType();
  const size_t len = att.getAttLength();

  try {
    if (type == netCDF::ncChar) {
      std::string value;
      att.getValues(value);
      std::print("\"{}\"", value);
    } else if (type == netCDF::ncFloat && len == 1) {
      float value;
      att.getValues(&value);
      std::print("{}", value);
    } else if (type == netCDF::ncDouble && len == 1) {
      double value;
      att.getValues(&value);
      std::print("{}", value);
    } else if (type == netCDF::ncInt && len == 1) {
      int value;
      att.getValues(&value);
      std::print("{}", value);
    } else if (type == netCDF::ncShort && len == 1) {
      short value;
      att.getValues(&value);
      std::print("{}", value);
    } else {
      std::print("<{}, {} value(s)>", type.getName(), len);
    }
  } catch (const netCDF::exceptions::NcException &exception) {
    std::print("<unreadable: {}>", exception.what());
  }

  std::println();
}

bool inspectFile(const fs::path &path) {
  try {
    netCDF::NcFile file(path.string(), netCDF::NcFile::read);
    std::println("File: {}", path.string());

    std::println("  Dimensions:");
    for (const auto &[name, dim] : file.getDims())
      std::println("    {} = {}", name, dim.getSize());

    std::println("  Variables:");
    for (const auto &[name, var] : file.getVars()) {
      std::println("    {} ({})", name, var.getType().getName());

      auto dims = var.getDims();
      if (!dims.empty()) {
        std::print(" [");
        for (size_t i = 0; i < dims.size(); ++i) {
          std::print("{}", dims[i].getName());
          if (i + 1 < dims.size())
            std::print(", ");
        }
        std::print("]");
      }
      std::println();

      for (const auto &[attName, att] : var.getAtts())
        printAttribute(attName, att);
    }

    std::println();
    return true;

  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "NetCDF error [{}]: {}.", path.string(),
                 exception.what());
    return false;
  }
}

} // namespace

int runInspect(const std::vector<std::string> &args) {
  InspectOptions opts;
  try {
    opts = parseInspectArgs(args);
  } catch (const std::exception &exception) {
    std::println(stderr, "Arguments error: {}.", exception.what());
    return EXIT_FAILURE;
  }

  bool allOk = true;
  for (const auto &input : opts.inputs) {
    if (!inspectFile(input))
      allOk = false;
  }

  return allOk ? EXIT_SUCCESS : EXIT_FAILURE;
}
