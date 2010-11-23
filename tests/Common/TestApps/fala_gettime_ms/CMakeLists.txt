# Set sources
set(SRC fala_gettime_ms.cpp)

set(CMAKE_CXX_FLAGS "-O3")

# Link statically to minimize startup time.
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(fala_gettime_ms ${SRC})

# Install
install(PROGRAMS fala_gettime_ms DESTINATION /usr/bin/)

