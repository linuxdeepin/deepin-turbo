project(Applauncherd)

cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Set default C++ flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -g -Wno-long-long -O2 -Wextra -Wpointer-arith -Wwrite-strings -Wold-style-cast -Woverloaded-virtual -Wundef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -g -O2")

# By default only QtCore and QtGui modules are enabled
#set(QT_USE_QTDBUS TRUE)

# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

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

# Set the program name define. Must be at this level due to unit tests.
add_definitions(-DPROG_NAME="applauncherd")

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

# Install meegotouch-boostable.pc
install(FILES data/pkgconfig/meegotouch-boostable.pc DESTINATION /usr/lib/pkgconfig)

# Install the qmake config feature file
install(FILES data/mkspecs/features/meegotouch-boostable.prf DESTINATION /usr/share/qt4/mkspecs/features)

