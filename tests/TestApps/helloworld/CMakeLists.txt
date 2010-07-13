# Set sources
set(SRC helloworld.cpp)

link_libraries(${MEEGOTOUCH_LIBRARIES})

include(${QT_USE_FILE})
add_library(helloworld MODULE ${SRC})

# Install
install(PROGRAMS libhelloworld.so DESTINATION /usr/bin RENAME fala_ft_hello.launch)
install(PROGRAMS scripts/fala_ft_hello DESTINATION /usr/bin/)
install(PROGRAMS scripts/qhelloworld DESTINATION /usr/bin/)

# Install symlinks so that /usr/bin/helloworldX launches /usr/bin/helloworldX.launch
install(SCRIPT scripts/create_links.cmake)

