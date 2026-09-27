#ifndef CSV_WRITE_HPP
#define CSV_WRITE_HPP

#include <filesystem>

namespace fs = std::filesystem;

bool convertNcToCsv(fs::path &inputPath);

#endif
