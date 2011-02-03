include(FindX11)

# Qt support
include(${QT_USE_FILE})

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})

# Set sources
set(PLUGINSRC qdeclarativebooster.cpp pluginfactory.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)
set(LIBSRC mdeclarativecache.cpp mdeclarativecache.h mdeclarativecache_p.h)

# Set executables
add_library(mdeclarativecache SHARED ${LIBSRC})
set_target_properties(mdeclarativecache PROPERTIES
  VERSION 0.1 SOVERSION 0)
target_link_libraries(mdeclarativecache ${LIBDL} ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES})

add_library(qdeclarativebooster MODULE ${PLUGINSRC})
set_target_properties(qdeclarativebooster PROPERTIES 
  COMPILE_FLAGS -fvisibility=hidden)
target_link_libraries(qdeclarativebooster ${LIBDL} "-L. -lmdeclarativecache")
add_dependencies(qdeclarativebooster mdeclarativecache)

# Add install rule
install(TARGETS qdeclarativebooster DESTINATION /usr/lib/applauncherd/)
install(TARGETS mdeclarativecache DESTINATION /usr/lib)
install(FILES mdeclarativecache.h DESTINATION /usr/include/applauncherd
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
install(FILES README-QDECLARATIVEBOOSTER DESTINATION /usr/share/doc/applauncherd)

