module;
#include <netcdf>

export module cluster.writer;

import std;
import sst_schema;
import nc_utils;
import cluster.climatology;
import cluster.input;
import cluster.kmeans;

namespace fs = std::filesystem;

export class ClusterWriter {
public:
  explicit ClusterWriter(fs::path output, bool verbose = false)
      : m_output(std::move(output)), m_verbose(verbose) {}

  void write(const ClusterInput &input, const KMeans &kmeans) const {
    const std::size_t k = kmeans.k();
    const std::size_t num_pixels = kmeans.labels().size();
    constexpr std::size_t num_months = Climatology::NumMonths;

    if (kmeans.numCols() != num_months ||
        kmeans.centers().size() != k * num_months) {
      throw std::runtime_error(std::format(
          "cannot write clusters: centers do not match k x {} months.",
          num_months));
    }
    if (num_pixels == 0 || input.latitude().size() != num_pixels ||
        input.longitude().size() != num_pixels) {
      throw std::runtime_error(
          "cannot write clusters: labels do not match the pixel coordinates.");
    }

    std::vector<int> ids(num_pixels);
    std::iota(ids.begin(), ids.end(), 1);
    std::vector<int> centroid_ids(k);
    std::iota(centroid_ids.begin(), centroid_ids.end(), 0);
    std::vector<int> months(num_months);
    std::iota(months.begin(), months.end(), 1);

    ncGuard(m_output.string(), [&] {
      netCDF::NcFile out(m_output.string(), netCDF::NcFile::replace,
                         netCDF::NcFile::nc4);

      const netCDF::NcDim pixel_dim =
          out.addDim(std::string(schema::PixelDim), num_pixels);
      const netCDF::NcDim centroid_dim = out.addDim("centroid", k);
      const netCDF::NcDim month_dim = out.addDim("month", num_months);

      const netCDF::NcVar id_var = out.addVar("id", netCDF::ncInt, pixel_dim);
      const netCDF::NcVar lat_var = out.addVar(std::string(schema::LatitudeVar),
                                               netCDF::ncDouble, pixel_dim);
      const netCDF::NcVar lon_var = out.addVar(
          std::string(schema::LongitudeVar), netCDF::ncDouble, pixel_dim);
      const netCDF::NcVar cluster_var =
          out.addVar("cluster", netCDF::ncInt, pixel_dim);
      const netCDF::NcVar centroid_var =
          out.addVar("centroid", netCDF::ncInt, centroid_dim);
      const netCDF::NcVar month_var =
          out.addVar("month", netCDF::ncInt, month_dim);
      const netCDF::NcVar centers_var =
          out.addVar("centers", netCDF::ncFloat,
                     std::vector<netCDF::NcDim>{centroid_dim, month_dim});

      id_var.putAtt("long_name",
                    "pixel id (row number in the dataset, from 1)");
      lat_var.putAtt("units", "degrees_north");
      lat_var.putAtt("standard_name", "latitude");
      lat_var.putAtt("axis", "Y");
      lon_var.putAtt("units", "degrees_east");
      lon_var.putAtt("standard_name", "longitude");
      lon_var.putAtt("axis", "X");
      cluster_var.putAtt("long_name", "cluster of the pixel (0 to k-1)");
      centroid_var.putAtt("long_name", "cluster index");
      month_var.putAtt("long_name", "calendar month (1 = January)");
      centers_var.putAtt("long_name",
                         "cluster mean of the monthly SST climatology");
      centers_var.putAtt("units", "degree_Celsius");
      out.putAtt("title", "SST clusters: k-means on the monthly climatology");

      id_var.putVar(ids.data());
      lat_var.putVar(input.latitude().data());
      lon_var.putVar(input.longitude().data());
      cluster_var.putVar(kmeans.labels().data());
      centroid_var.putVar(centroid_ids.data());
      month_var.putVar(months.data());
      centers_var.putVar(kmeans.centers().data());
    });

    if (m_verbose) {
      std::println("wrote {} pixel(s) in {} cluster(s) to {}", num_pixels, k,
                   m_output.string());
    }
  }

private:
  fs::path m_output;
  bool m_verbose;
};
