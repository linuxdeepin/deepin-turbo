project(Applauncherd)

cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

#
# NOTE: For verbose build use VERBOSE=1
#

# Default C++-flags. Sub-builds might alter these.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -g -O3 -Wl,--as-needed")
# Default C-flags. Sub-builds might alter these.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -g -O3")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "/usr/lib/applauncherd/")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)


# Find libdl
find_library(LIBDL NAMES dl)

# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

if ($ENV{DEBUG_BUILD})
    add_definitions(-DDEBUG_BUILD)
endif ($ENV{DEBUG_BUILD})

# Set the program name defines. Must be at this level due to unit tests.
add_definitions(-DPROG_NAME_INVOKER="invoker")
add_definitions(-DPROG_NAME_LAUNCHER="applauncherd")
add_definitions(-DPROG_NAME_SINGLE_INSTANCE="single-instance")

# applauncherd will find lib*booster.so from here
add_definitions(-DBOOSTER_PLUGIN_DIR="/usr/lib/applauncherd")

# applauncherd will try to load single-instance using this path
add_definitions(-DSINGLE_INSTANCE_PATH="/usr/bin/single-instance")

# Disable debug logging, only error and warning messages get logged
# Currently effective only for invoker. Launcher part recognizes --debug
# which enables console echoing and debug messages.
add_definitions(-DDEBUG_LOGGING_DISABLED)

# Build with test coverage switch if BUILD_COVERAGE environment variable is set
if ($ENV{BUILD_COVERAGE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -DWITH_COVERAGE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -DWITH_COVERAGE")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} --coverage")
endif ($ENV{BUILD_COVERAGE})

# Sub build: applauncherd
add_subdirectory(src)

# Build tests if BUILD_TESTS environment variable is set
if ($ENV{BUILD_TESTS})
    # Sub build: tests
    # This will build the HelloWorld test library, unit tests, functional tests
    # and performance tests
    add_subdirectory(tests)
endif ($ENV{BUILD_TESTS})

# Target for documentation (make doc)
find_program(Doxygen NAMES doxygen)
if (EXISTS ${Doxygen})
    add_custom_target(doc COMMAND cd doc && ${Doxygen} doxygen-impldoc.conf COMMAND cd doc && ${Doxygen} doxygen-userdoc.conf COMMAND cd doc && ${Doxygen} doxygen-mdeclarativecache.conf)
else (EXISTS ${Doxygen})
    message(STATUS "Doxygen not found: you're not able to build documentation.")
endif (EXISTS ${Doxygen})

# Install html documentation
install(DIRECTORY doc/implementation-documentation DESTINATION /usr/share/doc/applauncherd OPTIONAL)
install(DIRECTORY doc/user-documentation DESTINATION /usr/share/doc/applauncherd OPTIONAL)
install(DIRECTORY doc/mdeclarativecache-documentation DESTINATION /usr/share/doc/applauncherd OPTIONAL)

# Install startup script
if ($ENV{DEBUG_BUILD})
    install(PROGRAMS src/scripts/applauncherd-debug DESTINATION /usr/bin/ RENAME applauncherd)
else ($ENV{DEBUG_BUILD})
    install(PROGRAMS src/scripts/applauncherd DESTINATION /usr/bin/)
endif ($ENV{DEBUG_BUILD})

# Install pkg-config and qmake feature files
install(FILES data/pkgconfig/qt-boostable.pc DESTINATION /usr/lib/pkgconfig)
install(FILES data/mkspecs/features/qt-boostable.prf DESTINATION /usr/share/qt4/mkspecs/features)
install(FILES data/pkgconfig/qdeclarative-boostable.pc DESTINATION /usr/lib/pkgconfig)
install(FILES data/mkspecs/features/qdeclarative-boostable.prf DESTINATION /usr/share/qt4/mkspecs/features)

