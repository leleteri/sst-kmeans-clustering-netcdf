cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER clang++)

# Optimised builds are the default: the dataset is several gigabytes.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

project(SST_Analysis VERSION 1.0 LANGUAGES CXX)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED IMPORTED_TARGET netcdf)

# The netCDF C++ bindings have no pkg-config file on every platform, so they
# are located manually and exposed as an imported target.
find_path(NETCDF_CXX_INCLUDE_DIR NAMES ncFile.h REQUIRED)
find_library(NETCDF_CXX_LIBRARY NAMES netcdf-cxx4 netcdf_c++4 REQUIRED)

add_library(netcdf_cxx4 UNKNOWN IMPORTED)
set_target_properties(netcdf_cxx4 PROPERTIES
    IMPORTED_LOCATION "${NETCDF_CXX_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NETCDF_CXX_INCLUDE_DIR}"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(sst ${SOURCES})
target_compile_features(sst PRIVATE cxx_std_23)
target_include_directories(sst PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(sst PRIVATE PkgConfig::NETCDF netcdf_cxx4)
