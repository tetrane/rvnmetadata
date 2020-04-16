# Try to find magic

set(MAGIC_ROOT "${MAGIC_ROOT}" CACHE STRING "Where to find magic")

unset(MAGIC_INCLUDE_DIR CACHE)
unset(MAGIC_LIBRARY CACHE)

# find magic
find_path(MAGIC_INCLUDE_DIR "magic.h"
    HINTS ${MAGIC_ROOT}
    PATH_SUFFIXES "include"
    )
mark_as_advanced(MAGIC_INCLUDE_DIR)

find_library(MAGIC_LIBRARY magic HINTS ${MAGIC_ROOT}
    PATH_SUFFIXES "lib")
mark_as_advanced(MAGIC_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(magic DEFAULT_MSG
    MAGIC_INCLUDE_DIR MAGIC_LIBRARY)

# create the target
if(NOT TARGET magic)
	get_filename_component(MAGIC_LIBRARY_EXT ${MAGIC_LIBRARY} EXT)

	if (${MAGIC_LIBRARY_EXT} MATCHES ".so")
		add_library(magic SHARED IMPORTED)
	elseif (${MAGIC_LIBRARY_EXT} MATCHES ".a")
		add_library(magic STATIC IMPORTED)
	else()
		message(FATAL_ERROR "Can't determine the type of the library")
	endif()

	set_target_properties(magic PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${MAGIC_INCLUDE_DIR}"
		IMPORTED_LOCATION "${MAGIC_LIBRARY}"
	)
endif()
