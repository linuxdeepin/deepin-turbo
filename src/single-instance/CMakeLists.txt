set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

# Set sources
set(SRC main.cpp  ${COMMON}/report.c)

# Find X11
pkg_check_modules(X11 x11 REQUIRED)

# Set include dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${X11_INCLUDE_DIRS} ${COMMON})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC")

# Link as position independent executable
set(CMAKE_EXE_LINKER_FLAGS "-pie -rdynamic")

# Set C flags because of report.c
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -fPIC")

# Set link libraries
link_libraries(${X11_LDFLAGS})

# Set target
add_executable(single-instance ${SRC})

# Add install rule
install(PROGRAMS single-instance DESTINATION /usr/bin/)
