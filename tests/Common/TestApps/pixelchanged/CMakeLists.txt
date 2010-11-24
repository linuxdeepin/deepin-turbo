# Set sources
set(SRC main.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")

# This is needed for MeeGo only?
pkg_check_modules(XI xi REQUIRED)
include_directories(${XI_INCLUDE_DIRS})

pkg_check_modules(X11 x11 REQUIRED)
include_directories(${X11_INCLUDE_DIRS})

pkg_check_modules(XTST xtst REQUIRED)
include_directories(${XTST_INCLUDE_DIRS})

link_libraries(${X11_LDFLAGS} ${XTST_LDFLAGS} ${XI_LDFLAGS})
add_executable(fala_pixelchanged ${SRC})

# Install
install(PROGRAMS fala_pixelchanged DESTINATION /usr/bin/)

