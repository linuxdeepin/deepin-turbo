set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(WRTBOOSTER ${CMAKE_HOME_DIRECTORY}/src/wrtbooster)

# Set sources
set(SRC ut_wrtbooster.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp ${WRTBOOSTER}/wrtbooster.cpp
${LAUNCHER}/socketmanager.cpp)

# Set moc headers
set(MOC_HDRS ut_wrtbooster.h ${WRTBOOSTER}/wrtbooster.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER} ${WRTBOOSTER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${MEEGOTOUCH_LIBRARIES} ${LIBCREDS})

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_wrtbooster ${SRC} ${MOC_SRC} )

# Install
install(PROGRAMS ut_wrtbooster DESTINATION /usr/share/applauncherd-tests/)

