export module arguments;

import std;

export class Arguments {
public:
  Arguments() = default;
  explicit Arguments(int argc, char **argv) {
    parseArgs(argc, argv);

    if (m_subcommand.empty() && !m_inputs.empty()) {
      throw std::invalid_argument(
          std::format("unknown subcommand '{}'", m_inputs.front()));
    } else if ((m_subcommand == BuildOpts || m_subcommand == ClusterOpts) &&
               m_inputs.empty()) {
      throw std::invalid_argument(
          std::format("'{}' need atleast 1 input file", m_subcommand));
    } else if (m_subcommand.empty() && m_inputs.empty()) {
      throw std::invalid_argument(std::format("no subcommand specified"));
    }
  }

  std::string_view subcommand() const { return m_subcommand; }
  const std::vector<std::string_view> &options() const { return m_options; }
  const std::vector<std::string_view> &inputs() const { return m_inputs; }

private:
  std::string_view m_subcommand;
  std::vector<std::string_view> m_options;
  std::vector<std::string_view> m_inputs;

  static constexpr std::string_view BuildOpts = "build";
  static constexpr std::string_view ClusterOpts = "cluster";

  void parseArgs(int argc, char **argv) {
    if (argc <= 1)
      return;

    std::span<char *> raw_cmd(argv, argc);
    auto raw_args = raw_cmd.subspan(1);

    for (auto arg_ptr : raw_args) {
      std::string_view arg(arg_ptr);

      if (arg.starts_with("-")) {
        m_options.push_back(arg);
      } else if (m_subcommand.empty() &&
                 (arg == BuildOpts || arg == ClusterOpts)) {
        m_subcommand = arg;
      } else {
        m_inputs.push_back(arg);
      }
    }
  }
};
