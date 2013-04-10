# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC qtbooster.cpp pluginfactory.cpp)

# Set libraries to be linked.
link_libraries("-L../launcherlib -lapplauncherd" ${LIBDL} ${QT_QTCORE_LIBRARY})

# Set executable
add_library(qtbooster MODULE ${SRC})
add_dependencies(qtbooster applauncherd)

# Add install rule
install(TARGETS qtbooster DESTINATION /usr/lib/applauncherd/)

