cmake_minimum_required(VERSION 3.20)
project(NetCDFConverter VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED IMPORTED_TARGET netcdf)   # this one's fine

find_path(NETCDF_CXX_INCLUDE_DIR NAMES ncFile.h REQUIRED)
find_library(NETCDF_CXX_LIBRARY NAMES netcdf-cxx4 netcdf_c++4 REQUIRED)

add_library(netcdf_cxx4 UNKNOWN IMPORTED)
set_target_properties(netcdf_cxx4 PROPERTIES
    IMPORTED_LOCATION "${NETCDF_CXX_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NETCDF_CXX_INCLUDE_DIR}"
)

add_executable(nc2csv src/main.cpp)
target_include_directories(nc2csv PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(nc2csv PRIVATE PkgConfig::NETCDF netcdf_cxx4)
