project(Applauncherd)

cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Default C++-flags. Sub-builds might alter these.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -g -O2 -Wl,--as-needed")
# Default C-flags. Sub-builds might alter these.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -O2")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Find libdl
find_library(LIBDL NAMES dl)

# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

# Find libmeegotouch for MBooster, WRTBooster and the test programs
include(FindPkgConfig) 
pkg_check_modules(MEEGOTOUCH meegotouch>=0.20 REQUIRED)
include_directories(${MEEGOTOUCH_INCLUDE_DIRS})

# Check if it's possible to compile MBooster with MComponentCache. If so,
# HAVE_MCOMPONENTCACHE is set.
message(STATUS "checking for mcomponentcache.h")
find_file(M_COMPONENT_CACHE NAMES mcomponentcache.h PATHS ${MEEGOTOUCH_INCLUDE_DIRS})
if (EXISTS ${M_COMPONENT_CACHE})
  message(STATUS "  found")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_MCOMPONENTCACHE")
else (EXISTS ${M_COMPONENT_CACHE})
  message(STATUS "  not found")
endif (EXISTS ${M_COMPONENT_CACHE})

# Find creds.h (and libcreds if the header is found)
message(STATUS "checking for creds.h")
find_file(CREDS_H NAMES creds.h PATHS /usr/include/sys)
if (EXISTS ${CREDS_H})
    message(STATUS "  found: " ${CREDS_H})
    message(STATUS "checking for libcreds")
    find_library(LIBCREDS NAMES creds)
    if (EXISTS ${LIBCREDS})
        add_definitions(-DHAVE_CREDS)
        link_libraries(${LIBCREDS})
	    message(STATUS "  found: " ${LIBCREDS})
    else (EXISTS ${LIBCREDS})
    	message(STATUS "  not found: Platform Security is disabled.")
    endif (EXISTS ${LIBCREDS})
else (EXISTS ${CREDS_H})
  	message(STATUS "  not found: Platform Security is disabled.")
endif (EXISTS ${CREDS_H})

if ($ENV{DISABLE_VERIFICATION})
    add_definitions(-DDISABLE_VERIFICATION)
endif ($ENV{DISABLE_VERIFICATION})

# Set the program name defines. Must be at this level due to unit tests.
add_definitions(-DPROG_NAME_INVOKER="invoker")
add_definitions(-DPROG_NAME_LAUNCHER="applauncherd")

# Set gconf key definitions. These are used by MonitorBooster to get
# notifications when language / theme changes.
add_definitions(-DMEEGOTOUCH_THEME_GCONF_KEY="/meegotouch/theme/name")
add_definitions(-DMEEGOTOUCH_LANGUAGE_GCONF_KEY="/meegotouch/i18n/language")

# Disable debug logging, only error messages get logged
add_definitions(-DDEBUG_LOGGING_DISABLED)

# Build with test coverage switch if BUILD_COVERAGE environment variable is set
if ($ENV{BUILD_COVERAGE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftest-coverage -fprofile-arcs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage -fprofile-arcs")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -ftest-coverage -fprofile-arcs")
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
	add_custom_target(doc COMMAND ${Doxygen} doc/Doxyfile)
else (EXISTS ${Doxygen})
	message(STATUS "Doxygen not found: you're not able to build documentation.")
endif (EXISTS ${Doxygen})

# Install README
install(FILES README DESTINATION /usr/share/doc/applauncherd)

# Install startup script
install(PROGRAMS src/scripts/applauncherd DESTINATION /usr/bin/)

# Install pkg-config and qmake feature files. Note that current packaging policy is such 
# that applauncherd-dev installs only qt-boostable.pc which is intended for "all" 
# non-MeeGo Touch applications. meegotouch-boostable.pc is installed by libmeegotouch-dev 
# so as to make it easier for MeeGo Touch application developers. 
#
# TODO: Should meegotouch-boostable.* be removed completely from here since it's already included
#       in meegotouch-dev ?
install(FILES data/pkgconfig/meegotouch-boostable.pc DESTINATION /usr/lib/pkgconfig)
install(FILES data/mkspecs/features/meegotouch-boostable.prf DESTINATION /usr/share/qt4/mkspecs/features)
install(FILES data/pkgconfig/qt-boostable.pc DESTINATION /usr/lib/pkgconfig)
install(FILES data/mkspecs/features/qt-boostable.prf DESTINATION /usr/share/qt4/mkspecs/features)
if ($ENV{MEEGO})
    install(FILES meego/applauncherd.desktop DESTINATION /etc/xdg/autostart)
endif ($ENV{MEEGO})
