set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Find dbus
include(FindPkgConfig)
pkg_check_modules(DBUS dbus-1 REQUIRED)

# Set sources
set(SRC invokelib.c invoker.c ${COMMON}/report.c search.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${DBUS_INCLUDE_DIRS} ${COMMON})

# Set precompiler flags
add_definitions(-DPROG_NAME_INVOKER="invoker")

# Set target
add_executable(invoker ${SRC})

target_link_libraries(invoker ${DBUS_LDFLAGS})

# Add install rule
install(PROGRAMS invoker DESTINATION /usr/bin/)


