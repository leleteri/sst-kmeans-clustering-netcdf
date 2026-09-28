#ifndef DATA_VALIDATE_HPP
#define DATA_VALIDATE_HPP

#include <filesystem>
#include <vector>

bool validateGrid(const std::vector<std::filesystem::path> &files, bool verbose,
                  double tolerance = 1e-9);
bool validateStaticMask(const std::vector<std::filesystem::path> &files,
                        bool verbose);
#endif
