# Set sources
set(SRC main.cpp)

# This is needed for MeeGo only?
pkg_check_modules(XI xi REQUIRED)
include_directories(${XI_INCLUDE_DIRS})

pkg_check_modules(X11 x11 REQUIRED)
include_directories(${X11_INCLUDE_DIRS})

pkg_check_modules(XTST xtst REQUIRED)
include_directories(${XTST_INCLUDE_DIRS})

link_libraries(${X11_LDFLAGS} ${XTST_LDFLAGS} ${XI_LDFLAGS})
add_executable(fala_windowid ${SRC})

# Install
install(PROGRAMS fala_windowid DESTINATION /usr/bin/)

