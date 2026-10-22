#include "cluster/cluster_output.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <netcdf>
#include <numeric>
#include <print>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool writeClusterNc(const ClusterInput &input, const KMeansResult &result,
                    size_t k, const fs::path &output_path, bool verbose) {
  const std::string file_name = output_path.string();
  const size_t num_rows = result.labels.size();

  if (k == 0 || result.centers.empty() || result.centers.size() % k != 0) {
    std::println(stderr,
                 "Error: cannot write clusters: centers do not match k");
    return false;
  }
  const size_t num_months = result.centers.size() / k;

  if (num_rows == 0 || input.coords.lats.size() != num_rows ||
      input.coords.lons.size() != num_rows) {
    std::println(stderr, "Error: cannot write clusters: labels do not match "
                         "the pixel coordinates");
    return false;
  }
  const bool label_out_of_range =
      std::any_of(result.labels.begin(), result.labels.end(), [k](int label) {
        return label < 0 || static_cast<size_t>(label) >= k;
      });
  if (label_out_of_range) {
    std::println(stderr, "Error: cannot write clusters: a label is outside "
                         "0..k-1");
    return false;
  }

  // id is the row number of the pixel in the dataset, starting at 1
  std::vector<int> ids(num_rows);
  std::iota(ids.begin(), ids.end(), 1);
  std::vector<int> centroid_ids(k);
  std::iota(centroid_ids.begin(), centroid_ids.end(), 0);
  std::vector<int> months(num_months);
  std::iota(months.begin(), months.end(), 1);

  try {
    netCDF::NcFile out(file_name, netCDF::NcFile::replace, netCDF::NcFile::nc4);

    netCDF::NcDim pixel_dim = out.addDim("pixel", num_rows);
    netCDF::NcDim centroid_dim = out.addDim("centroid", k);
    netCDF::NcDim month_dim = out.addDim("month", num_months);

    netCDF::NcVar id_var = out.addVar("id", netCDF::ncInt, pixel_dim);
    netCDF::NcVar lat_var = out.addVar("latitude", netCDF::ncDouble, pixel_dim);
    netCDF::NcVar lon_var =
        out.addVar("longitude", netCDF::ncDouble, pixel_dim);
    netCDF::NcVar cluster_var = out.addVar("cluster", netCDF::ncInt, pixel_dim);
    netCDF::NcVar centroid_var =
        out.addVar("centroid", netCDF::ncInt, centroid_dim);
    netCDF::NcVar month_var = out.addVar("month", netCDF::ncInt, month_dim);

    std::vector<netCDF::NcDim> center_dims = {centroid_dim, month_dim};
    netCDF::NcVar centers_var =
        out.addVar("centers", netCDF::ncFloat, center_dims);

    id_var.putAtt("long_name", "pixel id (row number in the dataset, from 1)");
    lat_var.putAtt("units", "degrees_north");
    lon_var.putAtt("units", "degrees_east");
    cluster_var.putAtt("long_name", "cluster of the pixel (0 to k-1)");
    centroid_var.putAtt("long_name", "cluster index");
    month_var.putAtt("long_name", "calendar month (1 = January)");
    centers_var.putAtt("long_name",
                       "cluster mean of the monthly SST climatology");
    centers_var.putAtt("units", "degree_Celsius");
    out.putAtt("title", "SST clusters: k-means on the monthly climatology");

    id_var.putVar(ids.data());
    lat_var.putVar(input.coords.lats.data());
    lon_var.putVar(input.coords.lons.data());
    cluster_var.putVar(result.labels.data());
    centroid_var.putVar(centroid_ids.data());
    month_var.putVar(months.data());
    centers_var.putVar(result.centers.data());
  } catch (const netCDF::exceptions::NcException &exception) {
    std::println(stderr, "Error: {}: NetCDF: {}", file_name, exception.what());
    return false;
  } catch (const std::exception &exception) {
    std::println(stderr, "Error: {}: {}", file_name, exception.what());
    return false;
  }

  if (verbose)
    std::println("Wrote {} pixel(s) in {} cluster(s) to {}", num_rows, k,
                 file_name);
  return true;
}
