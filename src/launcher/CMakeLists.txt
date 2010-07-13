# Qt support
include(${QT_USE_FILE})

# Find Meego Touch libraries using pkg-config. 
include(FindPkgConfig) 
pkg_check_modules(MEEGOTOUCH meegotouch>=0.20 REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${MEEGOTOUCH_INCLUDE_DIRS})

# Check if it's possible to compile MBooster with MComponentCache. If so,
# HAVE_MCOMPONENTCACHE is set.
message(STATUS "checking for mcomponentcache.h")
find_file(M_COMPONENT_CACHE NAMES mcomponentcache.h PATHS ${MEEGOTOUCH_INCLUDE_DIRS})
if (EXISTS ${M_COMPONENT_CACHE})
    message(STATUS "  found")
    add_definitions(-DHAVE_MCOMPONENTCACHE)
else (EXISTS ${M_COMPONENT_CACHE})
    message(STATUS "  not found")
endif (EXISTS ${M_COMPONENT_CACHE})

# Set sources
set(SRC appdata.cpp booster.cpp connection.cpp daemon.cpp mbooster.cpp logger.cpp main.cpp qtbooster.cpp)

# Set the program name define used in daemon.cpp
add_definitions(-DPROG_NAME="applauncherd")

# Find libdl
find_library(LIBDL NAMES dl)

# Set libraries to be linked
link_libraries(${QT_LIBRARIES} ${MEEGOTOUCH_LIBRARIES} ${LIBDL})

# Set executable
add_executable(applauncherd.bin ${SRC})

# Add install rule
install(PROGRAMS applauncherd.bin scripts/applauncherd DESTINATION /usr/bin/)

