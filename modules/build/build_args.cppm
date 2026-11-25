export module build_args;

import std;

export class BuildArgs {
public:
  BuildArgs() = default;
  explicit BuildArgs(std::vector<std::string_view> inputs,
                     std::vector<std::string_view> opts) {
    build(inputs, opts);
    if (m_inputs.empty()) {
      throw std::invalid_argument(
          std::format("'build' need atleast 1 input file"));
    }
  }

  const bool &verbose() const noexcept { return m_verbose; }
  const std::size_t &limit() const noexcept { return m_limit; }
  const std::string_view &output() const noexcept { return m_output; }
  const std::vector<std::string_view> &inputs() { return m_inputs; }

private:
  bool m_verbose = false;
  std::size_t m_limit = 0;
  std::string_view m_output = "sst_dataset.nc";
  std::vector<std::string_view> m_inputs;

  static std::optional<std::size_t> to_size(std::string_view sv) {
    std::size_t result = 0;
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

    if (ec == std::errc() && ptr == sv.data() + sv.size()) {
      return result;
    }
    return std::nullopt;
  }

  void build(std::vector<std::string_view> inputs,
             std::vector<std::string_view> opts) {
    m_inputs = std::move(inputs);
    for (std::size_t i = 0; i < opts.size(); ++i) {
      std::string_view opt = opts[i];
      auto eq_pos = opt.find("=");
      if (eq_pos != std::string_view::npos) {
        std::string_view key = opt.substr(0, eq_pos);
        std::string_view val = opt.substr(eq_pos + 1);
        if (key == "-limit") {
          auto limit = to_size(val);
          if (!limit) {
            throw std::invalid_argument(
                std::format("value of limit must be a positive number!"));
          }
          m_limit = std::move(*limit);
        } else if (key == "-output") {
          m_output = std::move(val);
        } else if (key.starts_with('-')) {
          throw std::invalid_argument(std::format("invalid option '{}'", key));
        } else {
          throw std::invalid_argument(
              std::format("invalid argument '{}'", key));
        }
      } else {
        if (opt == "-v" || opt == "-verbose") {
          m_verbose = true;
        } else if (opt == "-limit" || opt == "-output") {
          throw std::invalid_argument(
              std::format("option '{}' require a value!", opt));
        } else if (opt.starts_with('-')) {
          throw std::invalid_argument(std::format("invalid option '{}'", opt));
        } else {
          throw std::invalid_argument(
              std::format("invalid argument '{}'", opt));
        }
      }
    }
  }
};
