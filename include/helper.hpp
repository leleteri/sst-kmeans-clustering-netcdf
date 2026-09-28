#ifndef HELPER_HPP
#define HELPER_HPP
#include "filename_parse.hpp"
#include "sst_data.hpp"
#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

std::vector<fs::path> collectNcFiles(const std::vector<std::string> &args);
std::optional<TimeAxis> buildTimeAxis(const std::vector<fs::path> &files);
bool summarizeDataset(const SstDataset &dataset);

#endif
