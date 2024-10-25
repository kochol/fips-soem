#
# project: fips-soem
#

if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 3.0)
    # include the fips main cmake file
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

    fips_setup(PROJECT fips-soem)
endif()

fips_add_subdirectory(soem)
