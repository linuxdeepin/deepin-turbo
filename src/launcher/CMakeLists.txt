# Generate list of libraries for preload.h
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/preload-h-libraries.h
  COMMAND python2.6 ${CMAKE_HOME_DIRECTORY}/scripts/library-helper.py --preload-h-libraries
  DEPENDS ${CMAKE_HOME_DIRECTORY}/scripts/library-helper.py
)

# Generate linker command line fragment
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/additional-linked-libraries.ld
  COMMAND python2.6 ${CMAKE_HOME_DIRECTORY}/scripts/library-helper.py --linker-flags
  DEPENDS ${CMAKE_HOME_DIRECTORY}/scripts/library-helper.py
)

# Set sources
set(SRC launcher.c)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/src/common ${CMAKE_CURRENT_BINARY_DIR})

# Set C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_POSIX_C_SOURCE=200112L -Wl,--no-as-needed -Wl,@additional-linked-libraries.ld")

# Set precompiler flags
add_definitions(-DLIBRARIES_CONFIG_FILE="/etc/applauncherd.conf")
add_definitions(-DLAUNCHER_LIBRARY="/usr/lib/applauncherd/libapplauncherd.so")

link_libraries(${LIBDL})

# Set target
add_executable(applauncherd.bin 
               ${SRC} 
               ${CMAKE_CURRENT_BINARY_DIR}/preload-h-libraries.h
               ${CMAKE_CURRENT_BINARY_DIR}/additional-linked-libraries.ld)

# Add install rule
install(PROGRAMS applauncherd.bin DESTINATION /usr/bin/)

