# Set sources
set(SRC fala_gettime.c)

# Link statically not supported in meego.
#set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(fala_gettime ${SRC})

# Install
install(PROGRAMS fala_gettime DESTINATION /usr/bin/)

