# Set sources
set(SRC fala_gettime.c)

set(CMAKE_C_FLAGS "-W -Wall -O3")

# Link statically not supported in meego.
#set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(fala_gettime ${SRC})

# Install
install(PROGRAMS fala_gettime DESTINATION /usr/bin/)

