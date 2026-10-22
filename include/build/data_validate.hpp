#ifndef DATA_VALIDATE_HPP
#define DATA_VALIDATE_HPP

#include <cstddef>
#include <filesystem>
#include <vector>

// Checks that every file uses the same latitude and longitude axes.
bool validateGrid(const std::vector<std::filesystem::path> &files, bool verbose,
                  double tolerance = 1e-9);

#endif
