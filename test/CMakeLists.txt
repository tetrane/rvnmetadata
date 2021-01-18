cmake_minimum_required(VERSION 3.7)
project(test)

find_package(Boost 1.49 COMPONENTS
    unit_test_framework
    filesystem
)

if(NOT Boost_FOUND)
  message(WARNING "Boost not found, don't build tests")
  return()
endif(NOT Boost_FOUND)

set(SOURCE_TEST_DATA "${CMAKE_SOURCE_DIR}/test/test_data/")
set(BINARY_TEST_DATA "${CMAKE_BINARY_DIR}/test/test_data/")

message("Generating test data")
file(COPY ${SOURCE_TEST_DATA} DESTINATION ${BINARY_TEST_DATA})

# rvnmetadata_version

add_executable(test_version
  test_version.cpp
)

target_include_directories(test_version PRIVATE "../include")
target_include_directories(test_version PRIVATE "../src")

target_link_libraries(test_version
  PUBLIC
    Boost::boost
  PRIVATE
    rvnmetadata
    Boost::unit_test_framework
    Boost::filesystem
)

target_compile_definitions(test_version PRIVATE "BOOST_TEST_DYN_LINK")
target_compile_definitions(test_version PRIVATE "TEST_DATA=\"${BINARY_TEST_DATA}\"")

add_test(rvnmetadata::version test_version)


# rvnmetadata_metadata

add_executable(test_metadata
  test_metadata.cpp
)

target_include_directories(test_metadata PRIVATE "../include")
target_include_directories(test_metadata PRIVATE "../src")

target_link_libraries(test_metadata
  PUBLIC
    Boost::boost
  PRIVATE
    rvnmetadata
    Boost::unit_test_framework
    Boost::filesystem
)

target_compile_definitions(test_metadata PRIVATE "BOOST_TEST_DYN_LINK")
target_compile_definitions(test_metadata PRIVATE "TEST_DATA=\"${BINARY_TEST_DATA}\"")

add_test(rvnmetadata::metadata test_metadata)
