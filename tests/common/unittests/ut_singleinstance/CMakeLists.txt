set(LAUNCHER ${CMAKE_HOME_DIRECTORY}/src/launcherlib)
set(DESTINATION "/usr/share/applauncherd-tests/")

add_definitions(-DTEST_LIB_LOCK_PATH="${DESTINATION}libutwithlock.so")
add_definitions(-DTEST_LIB_LOCKUNLOCK_PATH="${DESTINATION}libutwithlockunlock.so")

# Set sources
set(SRC ut_singleinstance.cpp ${LAUNCHER}/singleinstance.cpp)

# Set moc headers
set(MOC_HDRS ut_singleinstance.h)

# Run moc
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

# Enable test library
set(QT_USE_QTTEST TRUE)

# Set include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${LAUNCHER})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} -ldl)

# Enable Qt (may not be needed, because already defined on higher level)
include(${QT_USE_FILE})

add_executable(ut_singleinstance ${SRC} ${MOC_SRC} )
add_library(utwithlock MODULE ut_library_withlock.cpp)
add_library(utwithlockunlock MODULE ut_library_withlockunlock.cpp)

# Install
install(PROGRAMS ut_singleinstance DESTINATION ${DESTINATION})
install(FILES libutwithlock.so libutwithlockunlock.so  DESTINATION ${DESTINATION})
