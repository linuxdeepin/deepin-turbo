# Set sources
set(SRC main.cpp)
link_libraries(${MEEGOTOUCH_LIBRARIES} -lX11)

add_executable(xsendevent ${SRC})

# Install
install(PROGRAMS xsendevent DESTINATION /usr/bin/)

