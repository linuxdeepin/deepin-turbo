set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(EBOOSTER ${CMAKE_HOME_DIRECTORY}/src/ebooster)
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Set sources
set(SRC ut_ebooster.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/connection.cpp
${LAUNCHER}/logger.cpp ${EBOOSTER}/ebooster.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)

# Set moc headers
set(MOC_HDRS ut_ebooster.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER} ${EBOOSTER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${MEEGOTOUCH_LIBRARIES} ${LIBCREDS})

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_ebooster ${SRC} ${MOC_SRC} )

# Install
install(PROGRAMS ut_ebooster DESTINATION /usr/share/applauncherd-tests/)

