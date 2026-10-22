#ifndef CLUSTER_OUTPUT_HPP
#define CLUSTER_OUTPUT_HPP

#include "cluster/cluster_data.hpp"
#include "cluster/kmeans.hpp"
#include <cstddef>
#include <filesystem>

bool writeClusterNc(const ClusterInput &input, const KMeansResult &result,
                    size_t k, const std::filesystem::path &output_path,
                    bool verbose);

#endif
