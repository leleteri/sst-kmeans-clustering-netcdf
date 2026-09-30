#include "commands.hpp"
#include <cstdlib>
#include <iostream>

static void printUsage() {
  std::cerr << "Usage: sst <command> [options]\n\n"
               "Commands:\n"
               "  build    read .nc files and write the dataset file\n"
               "  cluster  cluster a dataset file\n";
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printUsage();
    return EXIT_FAILURE;
  }

  const std::string command = argv[1];
  const std::vector<std::string> rest(argv + 2, argv + argc);

  if (command == "build")
    return runBuild(rest);
  if (command == "cluster")
    return runCluster(rest);
  if (command == "-h" || command == "--help") {
    printUsage();
    return EXIT_SUCCESS;
  }

  std::cerr << "Unknown command: " << command << ".\n";
  printUsage();
  return EXIT_FAILURE;
}
