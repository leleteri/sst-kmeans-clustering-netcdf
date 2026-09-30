#include "arguments.hpp"
#include "cluster/climatology.hpp"
#include "cluster/cluster_data.hpp"
#include "cluster/kmeans.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

int runCluster(const std::vector<std::string> &args) {
  ClusterOptions opts;
  try {
    opts = parseClusterArgs(args);
  } catch (const std::exception &exception) {
    std::cerr << "Argument error: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }

  auto input = readClusterInput(opts.input);
  if (!input) {
    std::cerr << "Failed to read dataset: " << opts.input << "\n";
    return EXIT_FAILURE;
  }
  if (opts.verbose)
    std::cout << "Loaded " << input->num_rows << " pixel(s) x "
              << input->num_cols << " day(s) from " << opts.input << "\n";

  Climatology clim = buildClimatology(*input);
  if (opts.verbose)
    std::cout << "Built climatology: " << clim.num_rows << " pixel(s) x "
              << Climatology::num_months << " month(s)\n";

  KMeansResult result;
  try {
    result = runKMeans(clim.matrix, clim.num_rows, Climatology::num_months,
                       opts.k, opts.seed);
  } catch (const std::exception &exception) {
    std::cerr << "Clustering failed: " << exception.what() << "\n";
    return EXIT_FAILURE;
  }

  if (opts.verbose) {
    std::vector<size_t> counts(opts.k, 0);
    for (int label : result.labels)
      counts[static_cast<size_t>(label)] += 1;

    std::cout << "Cluster sizes:\n";
    for (size_t c = 0; c < opts.k; ++c)
      std::cout << "  cluster " << c << ": " << counts[c] << " pixel(s)\n";
  }

  // TODO: write the label map (lat, lon, cluster) and the cluster monthly
  // series once those export functions exist. For now, this only reports
  // what clustering produced -- nothing is saved to disk yet.
  std::cout << "Clustering complete. k=" << opts.k << ", seed=" << opts.seed
            << ".\n";

  return EXIT_SUCCESS;
}
