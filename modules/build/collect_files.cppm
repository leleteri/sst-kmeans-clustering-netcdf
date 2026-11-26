export module build.collect_files;

import std;

namespace fs = std::filesystem;

export std::vector<fs::path>
collectNcFiles(std::span<const std::string> inputs) {
  std::vector<fs::path> files;

  for (const std::string &input : inputs) {
    const fs::path path = fs::path(input).lexically_normal();

    if (fs::is_directory(path)) {
      for (const auto &entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".nc")
          files.push_back(entry.path());
      }
    } else if (fs::is_regular_file(path)) {
      if (path.extension() != ".nc") {
        std::println(std::cerr, "warning: {} does not have a .nc extension.",
                     path.string());
      }
      files.push_back(path);
    } else {
      std::println(std::cerr, "warning: skipping (not a file or directory): {}",
                   path.string());
    }
  }

  std::ranges::sort(files, [](const fs::path &a, const fs::path &b) {
    if (a.filename() != b.filename())
      return a.filename() < b.filename();
    return a < b;
  });
  const auto duplicates = std::ranges::unique(files);
  files.erase(duplicates.begin(), duplicates.end());
  return files;
}
