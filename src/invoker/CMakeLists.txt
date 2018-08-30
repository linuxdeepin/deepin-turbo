set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Find dbus
include(FindPkgConfig)
pkg_check_modules(DBUS dbus-1 REQUIRED)

# Set sources
set(SRC invokelib.c invoker.c ${COMMON}/report.c search.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${DBUS_INCLUDE_DIRS} ${COMMON})

# Set precompiler flags
add_definitions(-DPROG_NAME_INVOKER="deepin-turbo-invoker")

# Set target
add_executable(deepin-turbo-invoker ${SRC})

target_link_libraries(deepin-turbo-invoker ${DBUS_LDFLAGS})

# Add install rule
install(PROGRAMS deepin-turbo-invoker DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)


