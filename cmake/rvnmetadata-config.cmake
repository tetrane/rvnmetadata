get_filename_component(RVNMETADATA_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

find_dependency(magic REQUIRED)
find_dependency(rvnsqlite REQUIRED)
find_dependency(rvnbinresource REQUIRED)
find_dependency(rvnjsonresource REQUIRED)
find_package(Boost 1.49 COMPONENTS filesystem REQUIRED)

if(NOT TARGET rvnmetadata)
  include("${RVNMETADATA_CMAKE_DIR}/rvnmetadata-targets.cmake")
endif()
