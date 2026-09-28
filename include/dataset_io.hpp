#ifndef DATASET_IO_HPP
#define DATASET_IO_HPP

#include "sst_data.hpp"
#include <filesystem>

namespace fs = std::filesystem;

bool writeDatasetNc(const SstDataset &dataset, const fs::path &output_path,
                    bool verbose);

#endif
