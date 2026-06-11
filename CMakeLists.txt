cmake_minimum_required(VERSION 3.10)
project(ccserver VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

# ccoreconf (from submodule)
set(CCORECONF_DIR ${PROJECT_SOURCE_DIR}/external/ccoreconf)
if(EXISTS ${CCORECONF_DIR}/CMakeLists.txt)
    message(STATUS "Found ccoreconf submodule: ${CCORECONF_DIR}")

    # Pass nanocbor paths to ccoreconf
    set(NANOCBOR_INCLUDE ${NANOCBOR_INCLUDE_DIR})
    set(NANOCBOR_BUILD ${NANOCBOR_BUILD_DIR})

    # Don't build ccoreconf examples
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build ccoreconf examples" FORCE)

    add_subdirectory(${CCORECONF_DIR})
else()
    message(FATAL_ERROR "ccoreconf not found in external/ccoreconf. Run: git submodule update --init")
endif()

# Code generation from .sid file(s)
set(SID_FILES "" CACHE STRING "List of .sid files for code generation (semicolon-separated)")
set(GENERATED_NAME "generated" CACHE STRING "Base name for generated files")

# Generate handlers from SID files
if(SID_FILES)
    message(STATUS "Code generation enabled for: ${SID_FILES} to ${GENERATED_NAME}")

    set(SID_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

    # Handler wrapper files (auto-generated, included in build)
    set(SID_HANDLER_HEADER ${SID_GENERATED_DIR}/${GENERATED_NAME}-handlers.h)
    set(SID_HANDLER_SOURCE ${SID_GENERATED_DIR}/${GENERATED_NAME}-handlers.c)

    # Find Python3
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    # Create generated directory
    file(MAKE_DIRECTORY ${SID_GENERATED_DIR})

    # Custom command to run generateStubs.py
    # Generates: *-handlers.h, *-handlers.c (auto-included in build)
    #           *-impl.h, *-impl.c (user copies to src/ and modifies)
    add_custom_command(
        OUTPUT ${SID_HANDLER_HEADER} ${SID_HANDLER_SOURCE}
        COMMAND ${Python3_EXECUTABLE}
            ${CCORECONF_DIR}/tools/generateStubs.py
            ${GENERATED_NAME}
            -f ${SID_FILES}
        DEPENDS ${SID_FILES} ${CCORECONF_DIR}/tools/generateStubs.py
        WORKING_DIRECTORY ${SID_GENERATED_DIR}
        COMMENT "Generating handler wrappers from .sid files"
        VERBATIM
    )

    # Add generated files as a custom target
    add_custom_target(generate_handlers
        DEPENDS ${SID_HANDLER_HEADER} ${SID_HANDLER_SOURCE}
    )

    set(HANDLER_SOURCES ${SID_HANDLER_SOURCE})
    set(SID_GENERATED_INCLUDES ${SID_GENERATED_DIR})
else()
    set(HANDLER_SOURCES "")
    set(SID_GENERATED_INCLUDES "")
endif()

# Generate initial datastore from instance .json
if(INSTANCE_JSON AND SID_FILES) 
    message(STATUS "Code generation enabled for: ${INSTANCE_JSON} to ${GENERATED_NAME}")

    set(INSTANCE_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
    set(INSTANCE_GENERATED_HEADER ${INSTANCE_GENERATED_DIR}/${GENERATED_NAME}-instance.h)

    # Find Python3
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    # Create generated directory
    file(MAKE_DIRECTORY ${INSTANCE_GENERATED_DIR})

    # Custom command to run generateCBORDumps.py
    add_custom_command(
        OUTPUT ${INSTANCE_GENERATED_HEADER}
        COMMAND ${Python3_EXECUTABLE}
            ${CCORECONF_DIR}/tools/generateCBORDumps.py
            -i ${INSTANCE_JSON}
            -s ${SID_FILES}
            -o ${INSTANCE_GENERATED_HEADER}
        DEPENDS ${INSTANCE_JSON} ${CCORECONF_DIR}/tools/generateCBORDumps.py
        WORKING_DIRECTORY ${INSTANCE_GENERATED_DIR}
        COMMENT "Generating code from .json instance"
        VERBATIM
    )

    # Add generated files as a custom target
    add_custom_target(generate_instance
        DEPENDS ${INSTANCE_GENERATED_HEADER}
    )
else()
  set(INSTANCE_GENERATED_DIR "")
endif()

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/impl
    ${COAP_INCLUDE_DIRS}
    ${NANOCBOR_INCLUDE_DIR}
    ${CXXOPTS_INCLUDE_DIR}
    ${CCORECONF_DIR}/include
    ${INSTANCE_GENERATED_DIR}
)

# Source files
set(SOURCES
    src/main.cpp
    src/coreconf_handlers.cpp
    src/sid_handlers.c
    impl/simple-example-impl.c
    ${HANDLER_SOURCES}
)

# Create executable
add_executable(ccserver ${SOURCES})
add_dependencies(ccserver
  generate_instance
  generate_handlers
)

# Link libraries
target_link_libraries(ccserver
    ${COAP_LIBRARIES}
    nanocbor
    ccoreconf
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
message(STATUS "ccoreconf: ${CCORECONF_DIR}")
message(STATUS "cxxopts: ${CXXOPTS_INCLUDE_DIR}")
message(STATUS "====================================")
message(STATUS "")
