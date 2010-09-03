# Qt support
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Set sources
set(SRC appdata.cpp booster.cpp boosterfactory.cpp connection.cpp daemon.cpp mbooster.cpp logger.cpp main.cpp qtbooster.cpp wrtbooster.cpp)

# Find libdl
find_library(LIBDL NAMES dl)

# Set libraries to be linked
link_libraries(${QT_LIBRARIES} ${MEEGOTOUCH_LIBRARIES} ${LIBDL})

# Set executable
add_executable(applauncherd.bin ${SRC})

# Add install rule
install(PROGRAMS applauncherd.bin scripts/applauncherd DESTINATION /usr/bin/)

