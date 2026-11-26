import std;
import arguments;
import command;
import build.command;
import cluster.command;
import inspect.command;

namespace {

std::unique_ptr<Command> makeCommand(const Arguments &args) {
  switch (args.subcommand()) {
  case Subcommand::Build:
    return std::make_unique<BuildCommand>(args);
  case Subcommand::Cluster:
    return std::make_unique<ClusterCommand>(args);
  case Subcommand::Inspect:
    return std::make_unique<InspectCommand>(args);
  case Subcommand::Help:
    return std::make_unique<HelpCommand>();
  }
  throw std::logic_error("unhandled subcommand");
}

} // namespace

int main(int argc, char **argv) {
  try {
    const Arguments args(argc, argv);
    return makeCommand(args)->run();
  } catch (const ArgumentError &e) {
    // The user typed something wrong: show how to type it right.
    std::println(std::cerr, "error: {}\n", e.what());
    std::println(std::cerr, "{}", Usage);
    return ExitError;
  } catch (const std::exception &e) {
    std::println(std::cerr, "error: {}", e.what());
    return ExitError;
  }
}
