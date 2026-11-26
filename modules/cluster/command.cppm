export module cluster.command;

import std;
import arguments;
import command;
import cluster.arguments;
import cluster.climatology;
import cluster.input;
import cluster.kmeans;
import cluster.writer;

export class ClusterCommand final : public Command {
public:
  explicit ClusterCommand(const Arguments &args) : m_args(args) {}

  int run() override {
    const bool verbose = m_args.verbose();
    if (verbose) {
      std::println("clustering {} with k={}, seed={}", m_args.input(),
                   m_args.k(), m_args.seed());
    }

    const ClusterInput input(m_args.input());
    if (verbose) {
      std::println("loaded {} pixel(s) x {} day(s)", input.numPixels(),
                   input.numDays());
    }

    const Climatology climatology(input);
    if (const std::size_t empty = climatology.missingCells(); empty > 0) {
      throw std::runtime_error(std::format(
          "climatology has {} empty cell(s): some pixel has no data for a "
          "calendar month, so the dataset must cover all {} months.",
          empty, Climatology::NumMonths));
    }
    if (verbose) {
      std::println("built climatology: {} pixel(s) x {} month(s)",
                   climatology.numPixels(), Climatology::NumMonths);
    }

    KMeans kmeans(m_args.k(), m_args.seed());
    kmeans.fit(climatology.matrix(), climatology.numPixels(),
               Climatology::NumMonths);
    if (verbose)
      printClusters(kmeans);

    ClusterWriter(m_args.output(), verbose).write(input, kmeans);
    return ExitSuccess;
  }

private:
  ClusterArgs m_args;

  static void printClusters(const KMeans &kmeans) {
    std::println("k-means {} after {} iteration(s)",
                 kmeans.converged() ? "converged" : "stopped (iteration limit)",
                 kmeans.iterations());

    const std::vector<std::size_t> sizes = kmeans.clusterSizes();
    std::println("cluster sizes:");
    for (std::size_t c = 0; c < sizes.size(); ++c)
      std::println("  cluster {}: {} pixel(s)", c, sizes[c]);

    std::println("cluster centers (mean SST by month, January to December):");
    for (std::size_t c = 0; c < kmeans.k(); ++c) {
      std::string line;
      for (std::size_t month = 0; month < Climatology::NumMonths; ++month) {
        line += std::format(
            " {:.2f}", kmeans.centers()[c * Climatology::NumMonths + month]);
      }
      std::println("  cluster {}:{}", c, line);
    }
  }
};
