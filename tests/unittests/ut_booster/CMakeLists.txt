set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcher)

# Set sources
set(SRC ut_booster.cpp ${LAUNCHER}/appdata.cpp ${LAUNCHER}/booster.cpp ${LAUNCHER}/connection.cpp ${LAUNCHER}/logger.cpp)

# Set moc headers
set(MOC_HDRS ut_booster.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${LAUNCHER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${LIBCREDS})

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_booster ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS ut_booster DESTINATION /usr/share/applauncherd-tests/)

