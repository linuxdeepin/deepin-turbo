include(FindX11)

# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER} "${CMAKE_HOME_DIRECTORY}/src/common")

# Set sources
set(PLUGINSRC qdeclarativebooster.cpp pluginfactory.cpp)

set(LIBSRC mdeclarativecache.cpp mdeclarativecache.h mdeclarativecache_p.h eventhandler.cpp eventhandler.h)
set(MOC_HDRS eventhandler.h)
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Set executables
add_library(mdeclarativecache SHARED ${LIBSRC} ${MOC_SRC})
set_target_properties(mdeclarativecache PROPERTIES
  VERSION 0.1 SOVERSION 0)
if ($ENV{HARMATTAN})
  set_target_properties(mdeclarativecache PROPERTIES COMPILE_FLAGS -DHAVE_PATH_REINIT)
endif ($ENV{HARMATTAN})
target_link_libraries(mdeclarativecache ${LIBDL} ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTGUI_LIBRARY} ${X11_LIBRARIES} "-L../launcherlib -lapplauncherd")
add_dependencies(mdeclarativecache applauncherd)

add_library(qdeclarativebooster MODULE ${PLUGINSRC})
set_target_properties(qdeclarativebooster PROPERTIES 
  COMPILE_FLAGS -fvisibility=hidden)
target_link_libraries(qdeclarativebooster ${LIBDL} "-L. -lmdeclarativecache -L../launcherlib -lapplauncherd")
add_dependencies(qdeclarativebooster applauncherd mdeclarativecache)

# Add install rule
install(TARGETS qdeclarativebooster DESTINATION /usr/lib/applauncherd/)
install(TARGETS mdeclarativecache DESTINATION /usr/lib)
install(FILES mdeclarativecache.h MDeclarativeCache eventhandler.h DESTINATION /usr/include/applauncherd
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

