cmake_minimum_required(VERSION 3.10)
project(ccserver VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find libcoap (try different TLS backends)
pkg_check_modules(COAP libcoap-3-openssl)
if(NOT COAP_FOUND)
    pkg_check_modules(COAP libcoap-3-gnutls)
endif()
if(NOT COAP_FOUND)
    pkg_check_modules(COAP REQUIRED libcoap-3-notls)
endif()

# nanocbor (from submodule, built with meson)
include(ExternalProject)
set(NANOCBOR_DIR ${PROJECT_SOURCE_DIR}/external/NanoCBOR)
set(NANOCBOR_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/nanocbor-build)

if(EXISTS ${NANOCBOR_DIR}/meson.build)
    message(STATUS "Found nanocbor submodule: ${NANOCBOR_DIR}")

    # Build nanocbor using meson/ninja
    ExternalProject_Add(nanocbor_external
        SOURCE_DIR ${NANOCBOR_DIR}
        BINARY_DIR ${NANOCBOR_BUILD_DIR}
        CONFIGURE_COMMAND meson setup ${NANOCBOR_BUILD_DIR} ${NANOCBOR_DIR} --default-library=static
        BUILD_COMMAND ninja -C ${NANOCBOR_BUILD_DIR}
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS ${NANOCBOR_BUILD_DIR}/libnanocbor.a
    )

    # Create imported library target
    add_library(nanocbor STATIC IMPORTED)
    set_target_properties(nanocbor PROPERTIES
        IMPORTED_LOCATION ${NANOCBOR_BUILD_DIR}/libnanocbor.a
    )
    add_dependencies(nanocbor nanocbor_external)

    set(NANOCBOR_INCLUDE_DIR ${NANOCBOR_DIR}/include)
else()
    message(FATAL_ERROR "nanocbor not found in external/nanocbor. Run: git submodule update --init")
endif()

# cxxopts (from submodule)
set(CXXOPTS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/external/cxxopts/include)
if(EXISTS ${CXXOPTS_INCLUDE_DIR}/cxxopts.hpp)
    message(STATUS "Found cxxopts: ${CXXOPTS_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "cxxopts not found in external/cxxopts. Run: git submodule update --init")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${COAP_INCLUDE_DIRS}
    ${NANOCBOR_INCLUDE_DIR}
    ${CXXOPTS_INCLUDE_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/coreconf_handlers.cpp
)

# Create executable
add_executable(ccserver ${SOURCES})

# Link libraries
target_link_libraries(ccserver
    ${COAP_LIBRARIES}
    nanocbor
)

# Compiler options
target_compile_options(ccserver PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Installation rules
install(TARGETS ccserver DESTINATION bin)

# Print configuration summary
message(STATUS "")
message(STATUS "=== CORECONF Server Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "libcoap: ${COAP_VERSION}")
message(STATUS "nanocbor: ${NANOCBOR_DIR}")
message(STATUS "cxxopts: ${CXXOPTS_INCLUDE_DIR}")
message(STATUS "====================================")
message(STATUS "")
