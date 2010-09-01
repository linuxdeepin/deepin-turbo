# Set sources
set(SRC helloworld.cpp)

link_libraries(${MEEGOTOUCH_LIBRARIES})

include(${QT_USE_FILE})

# Use the compiler and linker flags given in meegotouch-boostable.pc
# in the source tree.
execute_process(COMMAND "env" 
                         "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                         "/usr/bin/pkg-config" 
                         "--cflags" 
                         "meegotouch-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_cflags 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "env" 
                        "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                        "/usr/bin/pkg-config" "--libs" 
                        "meegotouch-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_libs 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${booster_cflags}")
set(CMAKE_EXE_LINKER_FLAGS ${booster_libs})

add_executable(helloworld ${SRC})

# Install
install(PROGRAMS helloworld DESTINATION /usr/bin RENAME fala_ft_hello.launch)
install(PROGRAMS scripts/fala_ft_hello DESTINATION /usr/bin/)
install(PROGRAMS scripts/fala_ft_hello DESTINATION /usr/bin RENAME fala_wl)

# Install symlinks so that /usr/bin/helloworldX launches /usr/bin/helloworldX.launch
install(SCRIPT scripts/create_links.cmake)

