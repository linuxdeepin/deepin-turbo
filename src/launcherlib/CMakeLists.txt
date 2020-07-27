set(COMMON ${CMAKE_HOME_DIRECTORY}/src/common)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC appdata.cpp booster.cpp connection.cpp daemon.cpp logger.cpp
        singleinstance.cpp socketmanager.cpp)

set(HEADERS appdata.h booster.h connection.h daemon.h logger.h launcherlib.h
    singleinstance.h socketmanager.h ${COMMON}/protocol.h)

# Set libraries to be linked. Shared libraries to be preloaded are not linked in anymore,
# but dlopen():ed and listed in src/launcher/preload.h instead.
link_libraries(${LIBDL} "-L/lib -lsystemd")

# Set executable
add_library(deepin-turbo SHARED ${SRC} ${MOC_SRC})
set_target_properties(deepin-turbo PROPERTIES VERSION 0.1 SOVERSION 0)

if (NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

# Add install rule
install(TARGETS deepin-turbo DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/deepin-turbo
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
