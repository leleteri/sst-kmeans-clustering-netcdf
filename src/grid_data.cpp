#include "grid_data.hpp"
#include <netcdf>

namespace fs = std::filesystem;

std::optional<GridInfo> readGridInfo(const fs::path &inputPath) {
  try {
    netCDF::NcFile dataFile(inputPath.string(), netCDF::NcFile::read);
    netCDF::NcVar latVar = dataFile.getVar("latitude");
    netCDF::NcVar lonVar = dataFile.getVar("longitude");
    netCDF::NcVar sstVar = dataFile.getVar("analysed_sst");

    if (latVar.isNull() || lonVar.isNull() || sstVar.isNull()) {
      std::cerr << "Error [" << inputPath.filename()
                << "]: Required variables (latitude, longitude, analysed_sst) "
                   "not found.\n";
      return std::nullopt;
    }

    size_t numLats = latVar.getDim(0).getSize();
    size_t numLons = lonVar.getDim(0).getSize();

    GridInfo grid;
    grid.lats.resize(numLats);
    grid.lons.resize(numLons);
    latVar.getVar(grid.lats.data());
    lonVar.getVar(grid.lons.data());

    netCDF::NcVarAtt fillAtt = sstVar.getAtt("_FillValue");
    fillAtt.getValues(&grid.fillValue);

    std::vector<float> sst(numLats * numLons);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, numLats, numLons};
    sstVar.getVar(start, count, sst.data());

    grid.oceanMask.resize(numLats * numLons);
    for (size_t k = 0; k < sst.size(); ++k) {
      grid.oceanMask[k] = (sst[k] != grid.fillValue);
    }

    return grid;
  } catch (const netCDF::exceptions::NcException &exception) {
    std::cerr << "NetCDF Error [" << inputPath.filename() << "]"
              << exception.what();
    return std::nullopt;
  }
}

std::optional<std::vector<float>> readSstValues(const fs::path &inputPath,
                                                size_t expectedSize) {
  try {
    netCDF::NcFile dataFile(inputPath.string(), netCDF::NcFile::read);
    netCDF::NcVar sstVar = dataFile.getVar("analysed_sst");
    if (sstVar.isNull()) {
      std::cerr << "Error [" << inputPath.filename()
                << "]: analysed_sst not found.\n";
      return std::nullopt;
    }

    std::vector<float> sst(expectedSize);
    std::vector<size_t> start = {0, 0, 0};
    std::vector<size_t> count = {1, /* numLats */ 0,
                                 /* numLons */ 0}; // fill from grid dims
    sstVar.getVar(start, count, sst.data());
    return sst;
  } catch (const netCDF::exceptions::NcException &e) {
    std::cerr << "NetCDF Error [" << inputPath.filename() << "]: " << e.what()
              << "\n";
    return std::nullopt;
  }
}
