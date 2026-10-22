#ifndef BUILD_OUTPUT_HPP
#define BUILD_OUTPUT_HPP

#include "sst_data.hpp"
#include <filesystem>

bool writeDatasetNc(const SstDataset &dataset,
                    const std::filesystem::path &output_path, bool verbose);

#endif // BUILD_OUTPUT_HPP
