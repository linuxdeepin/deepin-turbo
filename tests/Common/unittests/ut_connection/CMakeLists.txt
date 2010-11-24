set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)

# Set sources
set(SRC ut_connection.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp ${LAUNCHER}/boosterfactory.cpp ${LAUNCHER}/daemon.cpp ${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp ${LAUNCHER}/mbooster.cpp ${LAUNCHER}/qtbooster.cpp ${LAUNCHER}/wrtbooster.cpp)

# Set moc headers
set(MOC_HDRS ut_connection.h ${LAUNCHER}/mbooster.h ${LAUNCHER}/wrtbooster.h)

# Set the program name define used in daemon.cpp
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPROG_NAME_LAUNCHER=\\\"applauncherd\\\"")

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})
link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${MEEGOTOUCH_LIBRARIES})

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})
add_executable(ut_connection ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS ut_connection DESTINATION /usr/share/applauncherd-tests/)

