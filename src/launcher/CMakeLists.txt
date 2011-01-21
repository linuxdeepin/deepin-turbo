# Set sources
set(SRC launcher.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common)

# Set C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_POSIX_C_SOURCE=200112L")

# Set precompiler flags
add_definitions(-DLIBRARIES_CONFIG_FILE="/etc/applauncherd.conf")
add_definitions(-DLAUNCHER_LIBRARY="/usr/lib/applauncherd/libapplauncherd.so")

link_libraries(${LIBDL})

# Set target
add_executable(applauncherd.bin ${SRC})

# Add install rule
install(PROGRAMS applauncherd.bin DESTINATION /usr/bin/)

