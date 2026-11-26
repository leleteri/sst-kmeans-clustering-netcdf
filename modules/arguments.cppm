export module arguments;

import std;

export class ArgumentError : public std::invalid_argument {
public:
  using std::invalid_argument::invalid_argument;
};

export enum class Subcommand { Build, Cluster, Inspect, Help };

export class Arguments {
public:
  Arguments(int argc, char **argv) {
    const std::span<char *> raw(argv, static_cast<std::size_t>(argc));
    if (raw.size() < 2)
      throw ArgumentError("no subcommand specified");

    const std::string_view first = raw[1];
    const auto subcommand = parseSubcommand(first);
    if (!subcommand)
      throw ArgumentError(std::format("unknown subcommand '{}'", first));
    m_subcommand = *subcommand;

    for (const char *arg : raw.subspan(2)) {
      const std::string_view text = arg;
      (text.starts_with('-') ? m_options : m_inputs).push_back(text);
    }
  }

  Subcommand subcommand() const noexcept { return m_subcommand; }
  const std::vector<std::string_view> &options() const noexcept {
    return m_options;
  }
  const std::vector<std::string_view> &inputs() const noexcept {
    return m_inputs;
  }

private:
  Subcommand m_subcommand = Subcommand::Help;
  std::vector<std::string_view> m_options;
  std::vector<std::string_view> m_inputs;

  static std::optional<Subcommand> parseSubcommand(std::string_view name) {
    if (name == "build")
      return Subcommand::Build;
    if (name == "cluster")
      return Subcommand::Cluster;
    if (name == "inspect")
      return Subcommand::Inspect;
    if (name == "help" || name == "-h" || name == "--help")
      return Subcommand::Help;
    return std::nullopt;
  }
};
