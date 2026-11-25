module;

export module collect_files;

import std;
import arguments;

namespace fs = std::filesystem;

export std::vector<fs::path>
collectNcFiles(const std::vector<std::string_view> &args) {
  std::vector<fs::path> files;
  for (const auto &arg : args) {
    fs::path p{arg};
    if (fs::is_directory(p)) {
      for (const auto &entry : fs::directory_iterator(p)) {
        if (entry.path().extension() == ".nc")
          files.push_back(entry.path());
      }
    } else if (fs::is_regular_file(p)) {
      if (p.extension() != ".nc") {
        std::println(std::cerr, "warning: {} does not have a .nc extension.",
                     p.string());
      }
      files.push_back(p);
    } else {
      std::println(std::cerr, "warning: skipping (not a file or directory): {}",
                   p.string());
    }
  }
  std::sort(files.begin(), files.end(),
            [](const fs::path &a, const fs::path &b) {
              return a.filename() < b.filename();
            });
  return files;
}
