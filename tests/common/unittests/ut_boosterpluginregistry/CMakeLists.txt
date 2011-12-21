set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(DESTINATION "/usr/share/applauncherd-tests/")

add_definitions(-DTEST_LIB_CREATE_PATH="${DESTINATION}libutwithcreate.so")
add_definitions(-DTEST_LIB_CREATETYPE_PATH="${DESTINATION}libutwithcreatetype.so")
add_definitions(-DTEST_LIB_CREATETYPESOCKETNAME_PATH="${DESTINATION}libutwithcreatetypesocketname.so")
add_definitions(-DTEST_LIB_PLUGIN_PATH="${DESTINATION}libutplugin.so")

# Set sources
set(SRC ut_boosterpluginregistry.cpp ${LAUNCHER}/boosterpluginregistry.cpp)

# Set moc headers
set(MOC_HDRS ut_boosterpluginregistry.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_boosterpluginregistry ${SRC} ${MOC_SRC})
add_library(utwithcreate MODULE ut_library_withcreate.cpp)
add_library(utwithcreatetype MODULE ut_library_withcreatetype.cpp)
add_library(utwithcreatetypesocketname MODULE ut_library_withcreatetypesocketName.cpp)
add_library(utplugin MODULE ut_library_plugin.cpp)

# Install
install(PROGRAMS ut_boosterpluginregistry DESTINATION ${DESTINATION})
install(FILES libutwithcreate.so libutwithcreatetype.so libutwithcreatetypesocketname.so libutplugin.so DESTINATION ${DESTINATION})
