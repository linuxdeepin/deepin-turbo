set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(PROTOCOL ${CMAKE_HOME_DIRECTORY}/src/common) 
set(DESTINATION "/usr/share/applauncherd-tests/")


# Set sources
set(SRC ut_appdata.cpp ${LAUNCHER}/appdata.cpp)

# Set moc headers
set(MOC_HDRS ut_appdata.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER} ${PROTOCOL})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_appdata ${SRC} ${MOC_SRC} )



# Install
install(PROGRAMS ut_appdata DESTINATION ${DESTINATION})

