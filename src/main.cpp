#include "commands.hpp"
#include <cstdlib>
#include <print>

static void printUsage() {
  std::println(
      stderr,
      "Usage: sst <command> [options]"
      "Commands:\n"
      "  inspect  print dimensions, variables, and attributes of .nc file(s)\n"
      "  build    read .nc files and write the dataset file\n"
      "  cluster  cluster a dataset file");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printUsage();
    return EXIT_FAILURE;
  }

  const std::string command = argv[1];
  const std::vector<std::string> rest(argv + 2, argv + argc);

  if (command == "inspect")
    return runInspect(rest);
  if (command == "build")
    return runBuild(rest);
  if (command == "cluster")
    return runCluster(rest);
  if (command == "-h" || command == "--help") {
    printUsage();
    return EXIT_SUCCESS;
  }

  std::println(stderr, "Unknown command: {}", command);
  printUsage();
  return EXIT_FAILURE;
}
