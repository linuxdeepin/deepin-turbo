# Set sources
set(SRC main.c)

set(CMAKE_CXX_FLAGS "-O3")

# Link statically to minimize startup time.
set(CMAKE_EXE_LINKER_FLAGS "-lX11 -lXtst")

add_executable(fala_pixelchanged ${SRC})

# Install
install(PROGRAMS fala_pixelchanged DESTINATION /usr/bin/)

