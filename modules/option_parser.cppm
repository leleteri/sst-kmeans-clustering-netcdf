export module option_parser;

import std;
import arguments;

export template <std::unsigned_integral T>
T parseNumber(std::string_view option, std::string_view text) {
  T value{};
  const char *first = text.data();
  const char *last = text.data() + text.size();
  const auto [ptr, ec] = std::from_chars(first, last, value);

  if (ec == std::errc::result_out_of_range) {
    throw ArgumentError(
        std::format("option '{}': value '{}' is too large", option, text));
  }
  if (text.empty() || ec != std::errc{} || ptr != last) {
    throw ArgumentError(std::format(
        "option '{}' expects a non-negative number, got '{}'", option, text));
  }
  return value;
}

export class OptionParser {
public:
  using FlagHandler = std::function<void()>;
  using ValueHandler = std::function<void(std::string_view)>;

  explicit OptionParser(std::string_view command) : m_command(command) {}

  OptionParser &flag(std::initializer_list<std::string_view> names,
                     FlagHandler handler) {
    m_rules.push_back({toStrings(names), false,
                       [h = std::move(handler)](std::string_view) { h(); }});
    return *this;
  }

  OptionParser &value(std::initializer_list<std::string_view> names,
                      ValueHandler handler) {
    m_rules.push_back({toStrings(names), true, std::move(handler)});
    return *this;
  }

  void parse(std::span<const std::string_view> options) const {
    for (const std::string_view raw : options) {
      const auto [name, value] = split(raw);
      const Rule *rule = find(name);

      if (!rule) {
        throw ArgumentError(
            std::format("unknown option '{}' for '{}'", raw, m_command));
      }
      if (rule->takes_value && !value) {
        throw ArgumentError(
            std::format("option '{}' requires a value: {}=<value>", raw, raw));
      }
      if (!rule->takes_value && value) {
        throw ArgumentError(
            std::format("option '{}' does not take a value", raw));
      }
      rule->handler(value.value_or(std::string_view{}));
    }
  }

private:
  struct Rule {
    std::vector<std::string> names;
    bool takes_value;
    ValueHandler handler;
  };

  std::string_view m_command;
  std::vector<Rule> m_rules;

  static std::vector<std::string>
  toStrings(std::initializer_list<std::string_view> names) {
    return {names.begin(), names.end()};
  }

  // "--limit=5" -> {"limit", "5"},  "-v" -> {"v", nullopt}
  static std::pair<std::string_view, std::optional<std::string_view>>
  split(std::string_view raw) {
    while (raw.starts_with('-'))
      raw.remove_prefix(1);

    const auto eq = raw.find('=');
    if (eq == std::string_view::npos)
      return {raw, std::nullopt};
    return {raw.substr(0, eq), raw.substr(eq + 1)};
  }

  const Rule *find(std::string_view name) const {
    for (const Rule &rule : m_rules) {
      if (std::ranges::find(rule.names, name) != rule.names.end())
        return &rule;
    }
    return nullptr;
  }
};
