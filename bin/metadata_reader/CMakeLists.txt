add_executable(metadata_reader
  metadata_reader.cpp
)

target_link_libraries(metadata_reader
  PUBLIC
    common
    file
    Boost::boost
  PRIVATE
    Boost::program_options
)

include(GNUInstallDirs)
install(TARGETS metadata_reader
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/reven/bin
)
