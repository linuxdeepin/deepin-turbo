# Set sources
set(SRC main.cpp)

# Use the compiler and linker flags given in meegotouch-boostable.pc
# in the source tree.
execute_process(COMMAND "env" 
                         "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                         "/usr/bin/pkg-config" 
                         "--cflags" 
                         "qdeclarative-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_cflags 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "env" 
                        "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                        "/usr/bin/pkg-config" "--libs" 
                        "qdeclarative-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_libs 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${booster_cflags}")

include_directories(${CMAKE_SOURCE_DIR}/src/qdeclarativebooster)

# Enable Qt-support
include(${QT_USE_FILE})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} "-L${CMAKE_SOURCE_DIR}/src/qdeclarativebooster ${booster_libs}")
add_executable(fala_qml_helloworld.launch ${SRC})

# Install
install(PROGRAMS fala_qml_helloworld.launch DESTINATION /usr/bin/)
install(PROGRAMS scripts/fala_qml_helloworld DESTINATION /usr/bin/)
install(FILES main.qml DESTINATION /usr/share/fala_qml_helloworld)
install(FILES fala_qml_helloworld_wl.desktop DESTINATION /usr/share/applications)
install(FILES fala_qml_helloworld_wol.desktop DESTINATION /usr/share/applications)

# Install symlinks so that /usr/bin/fala_qml_helloworldX launches /usr/bin/fala_qml_helloworldX.launch
install(SCRIPT scripts/create_links.cmake)
