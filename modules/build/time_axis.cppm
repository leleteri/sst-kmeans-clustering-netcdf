export module build.time_axis;

import std;

namespace fs = std::filesystem;
namespace chrono = std::chrono;

export class TimeAxis {
public:
  TimeAxis() = default;
  explicit TimeAxis(std::vector<fs::path> files, bool verbose = false) {
    build(std::move(files), verbose);
  }

  const std::vector<fs::path> &files() const noexcept { return m_files; }
  const std::vector<int> &dayOffsets() const noexcept { return m_day_offsets; }

  explicit operator bool() const noexcept {
    return !m_files.empty() && m_files.size() == m_day_offsets.size();
  }

private:
  std::vector<fs::path> m_files;
  std::vector<int> m_day_offsets;

  void build(std::vector<fs::path> files, bool verbose) {
    std::vector<std::pair<fs::path, int>> entries;
    entries.reserve(files.size());
    for (fs::path &file : files) {
      const int offset = daysSinceEpoch(parseDate(file));
      entries.emplace_back(std::move(file), offset);
    }

    std::ranges::stable_sort(entries, {}, &std::pair<fs::path, int>::second);

    m_files.reserve(entries.size());
    m_day_offsets.reserve(entries.size());
    for (auto &[file, offset] : entries) {
      m_files.push_back(std::move(file));
      m_day_offsets.push_back(offset);
    }

    std::size_t gaps = 0;
    for (std::size_t i = 1; i < m_day_offsets.size(); ++i) {
      const int delta = m_day_offsets[i] - m_day_offsets[i - 1];
      if (delta == 0) {
        throw std::runtime_error(std::format("duplicate date: {} and {}.",
                                             m_files[i - 1].string(),
                                             m_files[i].string()));
      }
      if (delta != 1) {
        std::println(std::cerr, "warning: date gap: {} -> {} ({} days).",
                     m_files[i - 1].string(), m_files[i].string(), delta);
        ++gaps;
      }
    }

    if (verbose) {
      std::println("time axis: {} file(s), {} irregular step(s).",
                   m_files.size(), gaps);
    }
  }

  static int parseField(std::string_view digits, std::size_t from,
                        std::size_t length) {
    const std::string_view field = digits.substr(from, length);
    int value = 0;
    const auto [ptr, ec] =
        std::from_chars(field.data(), field.data() + field.size(), value);
    if (ec != std::errc{} || ptr != field.data() + field.size()) {
      throw std::runtime_error(std::format("'{}' is not a number", field));
    }
    return value;
  }

  static chrono::year_month_day parseDate(const fs::path &path) {
    const std::string name = path.filename().string();
    const std::string stem = path.stem().string();
    if (stem.size() < 8) {
      throw std::runtime_error(std::format(
          "{}: file name must end with YYYYMMDD, e.g. sst_20240131.nc", name));
    }

    const std::string_view digits =
        std::string_view(stem).substr(stem.size() - 8);
    try {
      const int year = parseField(digits, 0, 4);
      const int month = parseField(digits, 4, 2);
      const int day = parseField(digits, 6, 2);

      if (month < 1 || month > 12 || day < 1 || day > 31)
        throw std::runtime_error("out of range");

      const chrono::year_month_day date{
          chrono::year{year}, chrono::month{static_cast<unsigned>(month)},
          chrono::day{static_cast<unsigned>(day)}};
      if (!date.ok())
        throw std::runtime_error("no such day");
      return date;
    } catch (const std::runtime_error &e) {
      throw std::runtime_error(
          std::format("{}: invalid date '{}' ({})", name, digits, e.what()));
    }
  }

  static int daysSinceEpoch(const chrono::year_month_day &date) {
    return static_cast<int>(chrono::sys_days{date}.time_since_epoch().count());
  }
};
