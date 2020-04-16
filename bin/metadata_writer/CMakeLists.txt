add_executable(metadata_writer
  metadata_writer.cpp
)

target_link_libraries(metadata_writer
  PUBLIC
    rvnmetadata
    Boost::boost
  PRIVATE
    Boost::program_options
)

include(GNUInstallDirs)
install(TARGETS metadata_writer
  RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/reven/internals
)
