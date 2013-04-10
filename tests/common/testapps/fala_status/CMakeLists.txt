# Set sources
set(SRC fala_status.cpp)

add_executable(fala_status ${SRC})

# Install
install(PROGRAMS fala_status DESTINATION /usr/bin/ )

