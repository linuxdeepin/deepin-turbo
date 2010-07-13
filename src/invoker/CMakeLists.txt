# Set sources
set(SRC invokelib.c invoker.c report.c search.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Set C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -O2")

# Set precompiler flags
add_definitions(-DPROG_NAME="invoker")

# Set target
add_executable(invoker ${SRC})

# Add install rule
install(PROGRAMS invoker DESTINATION /usr/bin/)


