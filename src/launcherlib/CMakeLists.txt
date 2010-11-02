# Qt support
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Set sources
set(SRC appdata.cpp booster.cpp boosterfactory.cpp monitorbooster.cpp connection.cpp daemon.cpp mbooster.cpp logger.cpp main.cpp qtbooster.cpp wrtbooster.cpp)
set(MOC_HDRS monitorbooster.h mbooster.h)
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Use webruntime if found
# this is necessary for wrt booster to work efficiently
pkg_check_modules(WRT webruntime)
if(WRT_FOUND)
  add_definitions(-DHAVE_WRT)
  include_directories(${WRT_INCLUDE_DIRS})
endif(WRT_FOUND)

# Set libraries to be linked. Shared libraries to be preloaded are not linked in anymore,
# but dlopen():ed and listed in src/launcher/preload.h instead.
# WRT_LIBRARIES is needed always as it contains the implementation of WRTComponentCache
link_libraries(${MEEGOTOUCH_LIBRARIES} ${LIBDL} ${QT_QTCORE_LIBRARY} ${WRT_LIBRARIES})

# Set executable
add_library(launcher MODULE ${SRC} ${MOC_SRC})

# Add install rule
install(FILES liblauncher.so DESTINATION /usr/bin/ RENAME applauncherd.launch)

