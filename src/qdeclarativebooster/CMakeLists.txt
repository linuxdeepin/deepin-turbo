include(FindX11)

# Qt support
include(${QT_USE_FILE})

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})

# Set sources
set(PLUGINSRC qdeclarativebooster.cpp pluginfactory.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)
set(LIBSRC qdeclarativeboostercache.cpp qdeclarativeboostercache.h qdeclarativeboostercache_p.h)

# Set executables
add_library(qdeclarativeboostercache SHARED ${LIBSRC})
set_target_properties(qdeclarativeboostercache PROPERTIES
  VERSION 0.1 SOVERSION 0)
target_link_libraries(qdeclarativeboostercache ${LIBDL} ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES})

add_library(qdeclarativebooster MODULE ${PLUGINSRC})
set_target_properties(qdeclarativebooster PROPERTIES 
  COMPILE_FLAGS -fvisibility=hidden)
target_link_libraries(qdeclarativebooster ${LIBDL} "-L. -lqdeclarativeboostercache")
add_dependencies(qdeclarativebooster qdeclarativeboostercache)

# Add install rule
install(TARGETS qdeclarativebooster DESTINATION /usr/lib/applauncherd/)
install(TARGETS qdeclarativeboostercache DESTINATION /usr/lib)
install(FILES qdeclarativeboostercache.h DESTINATION /usr/include/applauncherd
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
