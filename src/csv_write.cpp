#include "arguments.hpp"
#include "grid_data.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netcdf>

namespace fs = std::filesystem;

bool convertNcToCsv(const fs::path &inputPath) {
  fs::path outputPath = inputPath;
  outputPath.replace_extension(".csv");

  try {
    netCDF::NcFile dataFile(inputPath.string(), netCDF::NcFile::read);
    netCDF::NcVar latVar = dataFile.getVar("latitude");
    netCDF::NcVar lonVar = dataFile.getVar("longitude");
    netCDF::NcVar sstVar = dataFile.getVar("analysed_sst");

    if (latVar.isNull() || lonVar.isNull() || sstVar.isNull()) {
      std::cerr << "Error [" << inputPath.filename()
                << "]: Required variables (latitude, longitude, analysed_sst) "
                   "not found.\n";
      return false;
    }

    size_t numLats = latVar.getDim(0).getSize();
    size_t numLons = lonVar.getDim(0).getSize();

    std::vector<double> lats(numLats);
    std::vector<double> lons(numLons);
    std::vector<float> ssts(numLats *
                            numLons); // time dim is size 1, squeeze it out

    latVar.getVar(lats.data());
    lonVar.getVar(lons.data());

    // analysed_sst has shape (time=1, latitude, longitude); read the single
    // time slice
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, numLats, numLons};
    sstVar.getVar(start, count, ssts.data());

    float fillValue = -1.175494e+38f;
    netCDF::NcVarAtt fillAtt = sstVar.getAtt("_FillValue");
    if (!fillAtt.isNull()) {
      fillAtt.getValues(&fillValue);
    }

    std::ofstream csvFile(outputPath);
    if (!csvFile.is_open()) {
      std::cerr << "Error: Failed to create CSV file at " << outputPath << "\n";
      return false;
    }

    csvFile << "Latitude,Longitude,Temperature\n";

    for (size_t i = 0; i < numLats; ++i) {
      for (size_t j = 0; j < numLons; ++j) {
        size_t index = (i * numLons) + j;
        if (ssts[index] == fillValue && !config.showMissingValues) {
          continue;
        }
        csvFile << lats[i] << "," << lons[j] << "," << ssts[index] << "\n";
      }
    }

    csvFile.close();
    if (config.verbose)
      std::cout << "Converted: " << inputPath.filename() << " -> "
                << outputPath.filename() << "\n";
    return true;

  } catch (const netCDF::exceptions::NcException &exception) {
    std::cerr << "NetCDF Error [" << inputPath.filename()
              << "]: " << exception.what() << "\n";
    return false;
  }
}
