#ifndef DATA_VALIDATE_HPP
#define DATA_VALIDATE_HPP

#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

static bool sameAxis(const std::vector<double> &a, const std::vector<double> &b,
                     double tolerance);
bool validateGrid(const std::vector<fs::path> &files, double tolerance = 1e-9);
bool validateStaticMask(const std::vector<fs::path> &files);
#endif
