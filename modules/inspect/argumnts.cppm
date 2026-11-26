export module inspect.arguments;

import std;
import arguments;
import option_parser;

export class InspectArgs {
public:
  explicit InspectArgs(const Arguments &args)
      : m_inputs(args.inputs().begin(), args.inputs().end()) {
    OptionParser("inspect")
        .flag({"v", "verbose"}, [this] { m_verbose = true; })
        .parse(args.options());

    if (m_inputs.empty()) {
      throw ArgumentError("'inspect' needs at least one .nc file");
    }
  }

  bool verbose() const noexcept { return m_verbose; }
  const std::vector<std::string> &inputs() const noexcept { return m_inputs; }

private:
  bool m_verbose = false;
  std::vector<std::string> m_inputs;
};
