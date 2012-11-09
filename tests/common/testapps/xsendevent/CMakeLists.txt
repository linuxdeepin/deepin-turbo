# Set sources
set(SRC main.cpp)
link_libraries(-lX11 ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

add_executable(xsendevent ${SRC})

# Install
install(PROGRAMS xsendevent DESTINATION /usr/bin/)

