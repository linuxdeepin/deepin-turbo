set(LAUNCHER  ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(QTBOOSTER ${CMAKE_HOME_DIRECTORY}/src/qtbooster)

# Set sources
set(SRC ut_qtbooster.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp ${QTBOOSTER}/qtbooster.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)

# Set moc headers
set(MOC_HDRS ut_qtbooster.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER} ${QTBOOSTER})
link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${X11_LIBRARIES} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})
add_executable(ut_qtbooster ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS ut_qtbooster DESTINATION /usr/share/applauncherd-tests/)
