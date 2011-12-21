set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)

# Set sources
set(SRC ut_boosterfactory.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp
${LAUNCHER}/boosterfactory.cpp ${LAUNCHER}/boosterpluginregistry.cpp
${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp
${LAUNCHER}/singleinstance.cpp ${LAUNCHER}/socketmanager.cpp)

# Set moc headers
set(MOC_HDRS ut_boosterfactory.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${MEEGOTOUCH_LIBRARIES} ${LIBCREDS} ${X11_LIBRARIES} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_boosterfactory ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS ut_boosterfactory DESTINATION /usr/share/applauncherd-tests/)

