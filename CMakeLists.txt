cmake_minimum_required(VERSION 3.7)
project(rvnmetadata)

include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

option(BUILD_SHARED_LIBS "Set to ON to build shared libraries; OFF for static libraries." OFF)

option(WARNING_AS_ERROR "Set to ON to build with -Werror" ON)

option(BUILD_TEST_COVERAGE "Set to ON to build while generating coverage information. Will put source on the build directory." OFF)

find_package(magic PATHS ${CMAKE_SOURCE_DIR}/cmake REQUIRED)
find_package(rvnsqlite REQUIRED)
find_package(rvnbinresource REQUIRED)
find_package(rvnjsonresource REQUIRED)
find_package(Boost 1.49 COMPONENTS program_options filesystem REQUIRED)

add_library(rvnmetadata
  src/metadata.cpp
)

target_compile_options(rvnmetadata PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith
-Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(rvnmetadata PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(rvnmetadata PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(rvnmetadata PRIVATE gcov)
endif()

target_include_directories(rvnmetadata
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata.h
)

target_link_libraries(rvnmetadata
  PUBLIC
    rvnsqlite
    rvnbinresource
    rvnjsonresource
    Boost::boost
    magic
  PRIVATE
    Boost::filesystem
)

set_target_properties(rvnmetadata PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
)

include(GNUInstallDirs)
install(TARGETS rvnmetadata
  EXPORT rvnmetadata-export
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/reven/bin
)

install(EXPORT rvnmetadata-export
  FILE
    rvnmetadata-targets.cmake
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/cmake/rvnmetadata
)

install(FILES cmake/rvnmetadata-config.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/rvnmetadata
)

install(FILES cmake/magic-config.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/magic
)

add_subdirectory(bin)

enable_testing()
add_subdirectory(test)
