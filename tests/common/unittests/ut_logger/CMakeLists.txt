set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(COMMON ${CMAKE_HOME_DIRECTORY}/src/common)
set(DESTINATION "/usr/share/applauncherd-tests/")

# Set sources
set(SRC ut_logger.cpp ${LAUNCHER}/logger.cpp)

# Set moc headers
set(MOC_HDRS ut_logger.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER} ${COMMON})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_logger ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS ut_logger DESTINATION ${DESTINATION})

