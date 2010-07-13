# Set sources
set(SRC main.cpp)
link_libraries(${MEEGOTOUCH_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_EXE_LINKER_FLAGS "-pie -rdynamic") 

# Enable Qt-support
include(${QT_USE_FILE})
add_executable(fala_testapp ${SRC})

# Install
install(PROGRAMS fala_testapp DESTINATION /usr/bin/)
install(FILES com.nokia.fala_testapp.service DESTINATION /usr/share/dbus-1/services)
