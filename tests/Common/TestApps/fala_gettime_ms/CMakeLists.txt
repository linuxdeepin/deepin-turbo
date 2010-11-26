# Set sources
set(SRC fala_gettime_ms.c)

# Link statically not supported in meego 
#set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(fala_gettime_ms ${SRC})

# Install
install(PROGRAMS fala_gettime_ms DESTINATION /usr/bin/)

