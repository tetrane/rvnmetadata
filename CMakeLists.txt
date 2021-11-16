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

## common

add_library(common
  src/metadata-common.cpp
)

target_compile_options(common PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas
-Wpointer-arith -Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(common PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(common PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(common PRIVATE gcov)
endif()

target_include_directories(common
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata-common.h
)

set_target_properties(common PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
  PREFIX "librvnmetadata-"
)

## json

add_library(json
  src/metadata-json.cpp
)

target_compile_options(json PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith
-Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(json PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(json PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(json PRIVATE gcov)
endif()

target_include_directories(json
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata-json.h
)

target_link_libraries(json
  PUBLIC
    common
    rvnjsonresource
)

set_target_properties(json PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
  PREFIX "librvnmetadata-"
)

## sql

add_library(sql
  src/metadata-sql.cpp
)

target_compile_options(sql PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith
-Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(sql PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(sql PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(sql PRIVATE gcov)
endif()

target_include_directories(sql
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata-sql.h
)

target_link_libraries(sql
  PUBLIC
    common
    rvnsqlite
)

set_target_properties(sql PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
  PREFIX "librvnmetadata-"
)

## bin

add_library(bin
  src/metadata-bin.cpp
)

target_compile_options(bin PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith
-Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(bin PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(bin PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(bin PRIVATE gcov)
endif()

target_include_directories(bin
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata-bin.h
)

target_link_libraries(bin
  PUBLIC
    common
    rvnbinresource
)

set_target_properties(bin PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
  PREFIX "librvnmetadata-"
)

## file


add_library(file
  src/metadata-file.cpp
)

target_compile_options(file PRIVATE -W -Wall -Wextra -Wmissing-include-dirs -Wunknown-pragmas -Wpointer-arith
-Wmissing-field-initializers -Wno-multichar -Wreturn-type)

if(WARNING_AS_ERROR)
  target_compile_options(file PRIVATE -Werror)
endif()

if(BUILD_TEST_COVERAGE)
  target_compile_options(file PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
  target_link_libraries(file PRIVATE gcov)
endif()

target_include_directories(file
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(PUBLIC_HEADERS
  include/metadata-file.h
)

target_link_libraries(file
  PUBLIC
    common
    sql
    bin
    json
    Boost::boost
    magic
  PRIVATE
    Boost::filesystem
)

set_target_properties(file PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  POSITION_INDEPENDENT_CODE ON
  PREFIX "librvnmetadata-"
)

## install/exports

include(GNUInstallDirs)
install(TARGETS common bin sql json file
  EXPORT rvnmetadata-export
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/reven/bin
)

install(EXPORT rvnmetadata-export
  FILE
    rvnmetadata-targets.cmake
  NAMESPACE rvnmetadata::
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
