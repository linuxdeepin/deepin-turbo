# Qt support
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Set sources
set(SRC appdata.cpp booster.cpp boosterfactory.cpp boosterkiller.cpp connection.cpp daemon.cpp mbooster.cpp logger.cpp main.cpp qtbooster.cpp wrtbooster.cpp)
set(MOC_HDRS boosterkiller.h)
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Find libdl
find_library(LIBDL NAMES dl)

# Set libraries to be linked. Shared libraries to be preloaded are not linked in anymore,
# but dlopen():ed and listed in src/launcher/preload.h instead. 
link_libraries(${MEEGOTOUCH_LIBRARIES} ${LIBDL} ${QT_QTCORE_LIBRARY})

# Set executable
add_executable(applauncherd.bin ${SRC} ${MOC_SRC})

# Add install rule
install(PROGRAMS applauncherd.bin scripts/applauncherd DESTINATION /usr/bin/)

