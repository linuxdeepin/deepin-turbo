set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(DBOOSTER ${CMAKE_HOME_DIRECTORY}/src/qdeclarativebooster)

# Set sources
set(SRC ut_dbooster.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp ${DBOOSTER}/qdeclarativebooster.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)

# Set moc headers
set(MOC_HDRS ut_dbooster.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER} ${CMAKE_HOME_DIRECTORY}/src/common ${DBOOSTER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} "-L${CMAKE_SOURCE_DIR}/src/qdeclarativebooster -lmdeclarativecache" -ldl -lX11)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_dbooster ${SRC} ${MOC_SRC} )

# Install
install(PROGRAMS ut_dbooster DESTINATION /usr/share/applauncherd-tests/)

