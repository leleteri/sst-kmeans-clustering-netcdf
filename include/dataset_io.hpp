#ifndef DATASET_IO_HPP
#define DATASET_IO_HPP

#include "sst_data.hpp"
#include <filesystem>

bool writeDatasetNc(const SstDataset &dataset,
                    const std::filesystem::path &output_path, bool verbose);

#endif
