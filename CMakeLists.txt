# This file (c) 2016-2017 AlertAvert.com.  All rights reserved.

project(protobuf-examples)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

# Include common cmake utilities:
include(${COMMON_UTILS_DIR}/commons.cmake OPTIONAL RESULT_VARIABLE COMMONS_FILE)

# Conan Packaging support
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

##
# The INSTALL_DIR env variable must be set, unless -DINSTALL_DIR is used.
if(NOT DEFINED INSTALL_DIR)
    if(DEFINED ENV{INSTALL_DIR})
        set(INSTALL_DIR $ENV{INSTALL_DIR})
    else()
        message(FATAL_ERROR
                "INSTALL_DIR should be defined as an environment variable, or via -D notation")
    endif()
endif()

### Setting up common configuration variables

# Version number
set(RELEASE_MAJOR 0)
set(RELEASE_MINOR 1)
set(RELEASE_PATCH 0)
set(RELEASE_STRING "${RELEASE_MAJOR}.${RELEASE_MINOR}.${RELEASE_PATCH}")

if (${COMMONS_FILE} STREQUAL NOTFOUND)
    message(WARNING "Not using CMake utilities")
else ()
    get_build_id(BUILD_ID)
    if (BUILD_ID)
        string(concat RELEASE_STRING RELEASE_STRING "-" BUILD_ID)
    endif ()
endif ()
message(STATUS "Building Release: ${RELEASE_STRING} (${CMAKE_BUILD_TYPE})")

##
# Depending on the build type (Debug/Release) Google Logging
# generates a differently-named library.
# Both the value of CMAKE_BUILD_TYPE and the `build_type` setting
# in `~/.conan/profiles/default` need to match.
#
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(GLOG glogd)
    set(PROTOBUF protobufd)
else()
    set(GLOG glog)
    set(PROTOBUF protobuf)
endif()

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/tests)
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# Configure a header file to pass settings into source code.
configure_file(
    ${SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/version.h
)

# Building Protocol Buffers.
#
find_package(Protobuf REQUIRED)

if(${PROTOBUF_FOUND})
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_DIR}/demo.proto)
    message(STATUS "When generated, Protocol Buffer files will be in:\n"
        "\tHeaders: ${PROTO_HDRS}\n\tSources: ${PROTO_SRCS}\n"
        "protoc: ${PROTOBUF_PROTOC_EXECUTABLE}\n"
        "protoc lib: ${PROTOBUF_PROTOC_LIBRARY}\n"
        "protobuf lib: ${PROTOBUF_LIBRARY}"
)
else()
    message(FATAL_ERROR "Could not find Google Protocol Buffers libraries")
endif()

include_directories(
        ${INCLUDE_DIR}
        ${PROJECT_BINARY_DIR}
        ${CMAKE_BINARY_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
        ${PROTO_HDRS}
        ${INSTALL_DIR}/include
)

link_directories(
        ${INSTALL_DIR}/lib
        ${PROJECT_BINARY_DIR}/lib
)

# TODO: Your source files go here.
set(SOURCES
        ${PROTO_SRCS}
        ${SOURCE_DIR}/Sample.cpp
)

# TODO: the library dependencies go here
set(UTILS_LIBS
        ${GLOG}
        ${PROTOBUF}
        pthread
)

##
# Shared (dynamic) library will be built here.
#
#add_library(mylib SHARED
#        ${SOURCES}
#)
#target_link_libraries(distutils ${UTILS_LIBS})


if (DEFINED INSTALL_DIR)
    message(STATUS "Installing shared libraries to ${INSTALL_DIR}/lib "
                   "and include files to ${INSTALL_DIR}/include/${PROJECT_NAME}")
#    install(TARGETS your_libs DESTINATION ${INSTALL_DIR}/lib)

    # Generated configuration file, containing version and build number.
    install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION
            "${INSTALL_DIR}/include/${PROJECT_NAME}")

    # Install Library headers.
    install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${INSTALL_DIR}/include/${PROJECT_NAME}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

else ()
    message(WARNING "INSTALL_DIR is not defined, files will not be installed."
                    " Use -DINSTALL_DIR=/path/to/install to enable")
endif ()

# Unit tests build is defined in the tests/ folder
add_subdirectory(${TESTS_DIR})

#########
# Examples - demo execs/programs to show usage of the libraries/utilities.
#########

set(EXAMPLES_DIR ${SOURCE_DIR}/examples)

# Examples will use the latest build for the shared libraries
link_directories(
        ${CMAKE_BINARY_DIR}/lib
        ${INSTALL_DIR}/lib
)

##
# Demo
#
add_executable(demo
        ${EXAMPLES_DIR}/demo.cpp
        ${SOURCES}
)
target_link_libraries(demo ${UTILS_LIBS})
