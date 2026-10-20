#include "arguments.hpp"
#include "cluster/climatology.hpp"
#include "cluster/cluster_data.hpp"
#include "cluster/kmeans.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <print>
#include <vector>

int runCluster(const std::vector<std::string> &args) {
  ClusterOptions opts;
  try {
    opts = parseClusterArgs(args);
  } catch (const std::exception &exception) {
    std::println(stderr, "Argument error: {}.", exception.what());
    return EXIT_FAILURE;
  }

  auto input = readClusterInput(opts.input);
  if (!input) {
    std::println(stderr, "Failed to read dataset: {}.", opts.input);
    return EXIT_FAILURE;
  }
  if (opts.verbose)
    std::println("Loaded {} pixel(s) x {} day(s) from {}", input->num_rows,
                 input->num_cols, opts.input);

  Climatology clim = buildClimatology(*input);
  if (opts.verbose)
    std::println("Built climatology: {} pixel(s) x {} month(s).", clim.num_rows,
                 Climatology::num_months);

  KMeansResult result;
  try {
    result = runKMeans(clim.matrix, clim.num_rows, Climatology::num_months,
                       opts.k, opts.seed);
  } catch (const std::exception &exception) {
    std::println(stderr, "Clustering failed: {}.", exception.what());
    return EXIT_FAILURE;
  }

  if (opts.verbose) {
    std::vector<size_t> counts(opts.k, 0);
    for (int label : result.labels)
      counts[static_cast<size_t>(label)] += 1;

    std::println("Cluster sizes:");
    for (size_t c = 0; c < opts.k; ++c)
      std::println("  cluster {}:  pixel(s)", c, counts[c]);
  }

  // TODO: write the label map (lat, lon, cluster) and the cluster monthly
  // series once those export functions exist. For now, this only reports
  // what clustering produced -- nothing is saved to disk yet.
  std::println("Clustering complete. k={}, seed={}.", opts.k, opts.seed);

  return EXIT_SUCCESS;
}
