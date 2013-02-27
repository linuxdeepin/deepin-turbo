# Qt support
include(${QT_USE_FILE})

set(COMMON ${CMAKE_HOME_DIRECTORY}/src/common)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC appdata.cpp booster.cpp boosterfactory.cpp boosterpluginregistry.cpp
        connection.cpp daemon.cpp logger.cpp main.cpp 
        singleinstance.cpp socketmanager.cpp)

set(HEADERS appdata.h booster.h boosterfactory.h boosterpluginregistry.h
    connection.h daemon.h logger.h launcherlib.h singleinstance.h socketmanager.h
    ${COMMON}/protocol.h)

# Set libraries to be linked. Shared libraries to be preloaded are not linked in anymore,
# but dlopen():ed and listed in src/launcher/preload.h instead.
link_libraries(${LIBDL} ${X11_LIBRARIES} "-L/lib -lsystemd-daemon")

# Set executable
add_library(applauncherd MODULE ${SRC} ${MOC_SRC})
set_target_properties(applauncherd PROPERTIES VERSION 0.1 SOVERSION 0)

# Add install rule
install(TARGETS applauncherd DESTINATION /usr/lib)
install(FILES ${HEADERS} DESTINATION /usr/include/applauncherd
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
