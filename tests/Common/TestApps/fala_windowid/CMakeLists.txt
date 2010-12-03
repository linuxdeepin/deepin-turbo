# Set sources
set(SRC main.cpp)

pkg_check_modules(X11 x11 REQUIRED)
include_directories(${X11_INCLUDE_DIRS})

link_libraries(${X11_LDFLAGS})
add_executable(fala_windowid ${SRC})

# Install
install(PROGRAMS fala_windowid DESTINATION /usr/bin/)

