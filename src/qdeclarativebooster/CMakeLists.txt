include(FindX11)

# Qt support
include(${QT_USE_FILE})

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER})

# Set sources
set(PLUGINSRC qdeclarativebooster.cpp pluginfactory.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp ${COMMON}/eventhandler.cpp)
set(MOC_HDRS ${COMMON}/eventhandler.h)
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

set(LIBSRC mdeclarativecache.cpp mdeclarativecache.h mdeclarativecache_p.h)

# Set executables
add_library(mdeclarativecache SHARED ${LIBSRC})
set_target_properties(mdeclarativecache PROPERTIES
  VERSION 0.1 SOVERSION 0)
target_link_libraries(mdeclarativecache ${LIBDL} ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES})

add_library(qdeclarativebooster MODULE ${PLUGINSRC} ${MOC_SRC})
set_target_properties(qdeclarativebooster PROPERTIES 
  COMPILE_FLAGS -fvisibility=hidden)
target_link_libraries(qdeclarativebooster ${LIBDL} "-L. -lmdeclarativecache")
add_dependencies(qdeclarativebooster mdeclarativecache)

# Add install rule
install(TARGETS qdeclarativebooster DESTINATION /usr/lib/applauncherd/)
install(TARGETS mdeclarativecache DESTINATION /usr/lib)
install(FILES mdeclarativecache.h MDeclarativeCache DESTINATION /usr/include/applauncherd
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
install(FILES README-QDECLARATIVEBOOSTER DESTINATION /usr/share/doc/applauncherd)

