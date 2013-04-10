# Set sources
set(SRC fala_wait.cpp)

add_executable(fala_wait ${SRC})

# Install
install(PROGRAMS fala_wait DESTINATION /usr/bin/ )

