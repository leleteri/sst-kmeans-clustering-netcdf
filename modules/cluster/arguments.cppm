export module cluster.arguments;

import std;
import arguments;
import option_parser;

export class ClusterArgs {
public:
  explicit ClusterArgs(const Arguments &args) {
    OptionParser("cluster")
        .flag({"v", "verbose"}, [this] { m_verbose = true; })
        .value({"k", "k-value"},
               [this](std::string_view v) {
                 m_k = parseNumber<std::size_t>("k", v);
               })
        .value({"seed"},
               [this](std::string_view v) {
                 m_seed = parseNumber<unsigned>("seed", v);
               })
        .value({"o", "output"},
               [this](std::string_view v) { m_output = std::string(v); })
        .parse(args.options());

    if (args.inputs().size() != 1) {
      throw ArgumentError("'cluster' needs exactly one dataset file");
    }
    m_input = std::string(args.inputs().front());

    if (m_k == 0) {
      throw ArgumentError("option 'k' is required and must be greater than 0");
    }
  }

  bool verbose() const noexcept { return m_verbose; }
  std::size_t k() const noexcept { return m_k; }
  unsigned seed() const noexcept { return m_seed; }
  const std::string &input() const noexcept { return m_input; }
  const std::string &output() const noexcept { return m_output; }

private:
  bool m_verbose = false;
  std::size_t m_k = 0;
  unsigned m_seed = 42;
  std::string m_input;
  std::string m_output = "clusters.nc";
};
