set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Set sources
set(SRC invokelib.c invoker.c ${COMMON}/report.c search.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON})

# Set precompiler flags
add_definitions(-DPROG_NAME_INVOKER="invoker")

# Set target
add_executable(invoker ${SRC})

# Add install rule
install(PROGRAMS invoker DESTINATION /usr/bin/)


