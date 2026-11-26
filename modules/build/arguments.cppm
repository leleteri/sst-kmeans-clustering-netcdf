export module build.arguments;

import std;
import arguments;
import option_parser;

export class BuildArgs {
public:
  explicit BuildArgs(const Arguments &args)
      : m_inputs(args.inputs().begin(), args.inputs().end()) {
    OptionParser("build")
        .flag({"v", "verbose"}, [this] { m_verbose = true; })
        .value({"l", "limit"},
               [this](std::string_view v) {
                 m_limit = parseNumber<std::size_t>("limit", v);
               })
        .value({"o", "output"},
               [this](std::string_view v) { m_output = std::string(v); })
        .parse(args.options());

    if (m_inputs.empty()) {
      throw ArgumentError("'build' needs at least one file or directory");
    }
  }

  bool verbose() const noexcept { return m_verbose; }
  std::size_t limit() const noexcept { return m_limit; }
  const std::string &output() const noexcept { return m_output; }
  const std::vector<std::string> &inputs() const noexcept { return m_inputs; }

private:
  bool m_verbose = false;
  std::size_t m_limit = 0;
  std::string m_output = "sst_dataset.nc";
  std::vector<std::string> m_inputs;
};
