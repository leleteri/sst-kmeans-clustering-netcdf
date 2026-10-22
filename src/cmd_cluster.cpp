#include "arguments.hpp"
#include "cluster/climatology.hpp"
#include "cluster/cluster_data.hpp"
#include "cluster/cluster_output.hpp"
#include "cluster/kmeans.hpp"
#include "commands.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <print>
#include <string>
#include <vector>

int runCluster(const std::vector<std::string> &args) {
  ClusterOptions opts;
  try {
    opts = parseClusterArgs(args);
  } catch (const std::exception &exception) {
    std::println(stderr, "Error: Argument error: {}.", exception.what());
    return EXIT_FAILURE;
  }

  if (opts.verbose) {
    std::println("Clustering {} with k={}, seed={}", opts.input, opts.k,
                 opts.seed);
  }

  const auto input = readClusterInput(opts.input);
  if (!input) {
    return EXIT_FAILURE;
  }
  if (opts.verbose) {
    std::println("Loaded {} pixel(s) x {} day(s) from {}", input->num_rows,
                 input->num_cols, opts.input);
  }

  const Climatology climatology = buildClimatology(*input);
  if (opts.verbose) {
    std::println("Built climatology: {} pixel(s) x {} month(s)",
                 climatology.num_rows, Climatology::num_months);
  }

  const auto empty_cells =
      std::count_if(climatology.matrix.begin(), climatology.matrix.end(),
                    [](float value) { return std::isnan(value); });
  if (empty_cells > 0) {
    std::println(
        stderr,
        "Error: Climatology has {} empty cell(s): some pixel has no data "
        "for a calendar month, so the dataset must cover all 12 "
        "months.",
        empty_cells);
    return EXIT_FAILURE;
  }

  KMeansResult result;
  try {
    result = runKMeans(climatology.matrix, climatology.num_rows,
                       Climatology::num_months, opts.k, opts.seed);
  } catch (const std::exception &exception) {
    std::println(stderr, "Error: Clustering failed: {}.", exception.what());
    return EXIT_FAILURE;
  }

  if (opts.verbose) {
    std::vector<size_t> counts(opts.k, 0);
    for (const int label : result.labels) {
      if (label < 0 || static_cast<size_t>(label) >= opts.k)
        continue;
      counts[static_cast<size_t>(label)] += 1;
    }

    std::println("Cluster sizes:");
    for (size_t cluster = 0; cluster < opts.k; ++cluster) {
      std::println("  cluster {}: {} pixel(s)", cluster, counts[cluster]);
    }

    std::println("Cluster centers (mean SST by month, January to December):");
    for (size_t cluster = 0; cluster < opts.k; ++cluster) {
      std::string line;
      for (size_t month = 0; month < Climatology::num_months; ++month) {
        line += std::format(
            " {:.2f}",
            result.centers[cluster * Climatology::num_months + month]);
      }
      std::println("  cluster {}:{}", cluster, line);
    }
  }

  if (!writeClusterNc(*input, result, opts.k, opts.output, opts.verbose)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
