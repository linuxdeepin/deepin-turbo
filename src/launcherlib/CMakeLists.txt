# Qt support
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC appdata.cpp booster.cpp boosterfactory.cpp boosterpluginregistry.cpp
        connection.cpp daemon.cpp logger.cpp main.cpp 
        singleinstance.cpp socketmanager.cpp)

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
link_libraries(${LIBDL} ${QT_QTCORE_LIBRARY})

# Set executable
add_library(applauncherd MODULE ${SRC} ${MOC_SRC})

# Add install rule
install(FILES libapplauncherd.so DESTINATION /usr/lib/applauncherd/)

