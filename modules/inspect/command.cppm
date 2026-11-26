export module inspect.command;

import std;
import arguments;
import command;
import inspect.arguments;
import inspect.inspector;

export class InspectCommand final : public Command {
public:
  explicit InspectCommand(const Arguments &args) : m_args(args) {}

  int run() override {
    const NcInspector inspector(m_args.verbose());

    bool all_ok = true;
    for (const std::string &input : m_args.inputs()) {
      try {
        inspector.inspect(input);
      } catch (const std::exception &e) {
        std::println(std::cerr, "error: {}", e.what());
        all_ok = false;
      }
    }
    return all_ok ? ExitSuccess : ExitError;
  }

private:
  InspectArgs m_args;
};
