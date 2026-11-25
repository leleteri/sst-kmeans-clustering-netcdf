export module time_axis;

import std;

namespace fs = std::filesystem;
namespace df = std::chrono;

export class TimeAxis {
public:
  TimeAxis() = default;
  explicit TimeAxis(std::vector<fs::path> &files) { buildTimeAxis(files); }

  const std::vector<fs::path> &files() const { return m_files; }
  const std::vector<long> &dayOffsets() const { return m_day_offsets; }

  explicit operator bool() const {
    return (!m_files.empty() || !m_day_offsets.empty());
  }

private:
  std::vector<fs::path> m_files;
  std::vector<long> m_day_offsets;

  void buildTimeAxis(const std::vector<fs::path> &files) {
    std::vector<std::pair<fs::path, long>> entries;
    entries.reserve(files.size());

    for (const auto &file : files) {
      entries.emplace_back(file, daysSinceEpoch(parseDateFromFilename(file)));
    }

    std::stable_sort(
        entries.begin(), entries.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });

    m_files.reserve(entries.size());
    m_day_offsets.reserve(entries.size());
    for (const auto &[file, offset] : entries) {
      m_files.push_back(file);
      m_day_offsets.push_back(offset);
    }

    std::size_t gaps = 0;
    for (std::size_t i = 1; i < m_day_offsets.size(); ++i) {
      const long delta = m_day_offsets[i] - m_day_offsets[i - 1];
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

    std::println("time axis: {} files, {} irregular step(s).", m_files.size(),
                 gaps);
  }

  static int parse_field(std::string_view text, std::size_t from,
                         std::size_t to)

  {
    if (from > to || to > text.size())
      throw std::runtime_error("out of bound");

    std::string_view field = text.substr(from, to - from);

    int value{};
    auto [ptr, ec] =
        std::from_chars(field.data(), field.data() + field.size(), value);

    if (ec != std::errc{} || ptr != field.data() + field.size())
      throw std::runtime_error("parsing error");
    return value;
  }

  static df::year_month_day parseDateFromFilename(const fs::path &path) {
    const auto &raw_stem = path.stem().string();
    std::string_view stem = raw_stem;
    if (stem.size() < 8) {
      throw std::runtime_error(
          std::format("{}: file name format mismatch\n\tformat: sst_YYYMMDD.nc",
                      path.filename().string()));
    }

    std::string_view grep_date = stem.substr(stem.size() - 8);
    int year = parse_field(grep_date, 0, 4);
    int month = parse_field(grep_date, 4, 6);
    int day = parse_field(grep_date, 6, 8);

    if (month < 1 || month > 12 || day < 1 || day > 31) {
      throw std::runtime_error(std::format(
          "{}: invalid date '{}'", path.filename().string(), grep_date));
    }

    const df::year_month_day date{df::year{year},
                                  df::month{static_cast<unsigned>(month)},
                                  df::day{static_cast<unsigned>(day)}};
    if (!date.ok()) {
      std::runtime_error(std::format("{}: invalid date '{}'",
                                     path.filename().string(), grep_date));
    }

    return date;
  }

  static long daysSinceEpoch(const df::year_month_day &year_month_day) {
    return df::sys_days{year_month_day}.time_since_epoch().count();
  }
};
