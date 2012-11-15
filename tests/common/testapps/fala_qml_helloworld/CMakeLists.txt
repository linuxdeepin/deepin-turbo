# Set sources
set(SRC main.cpp)

# Use the compiler and linker flags given in qdeclarative-boostable.pc
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

# Compile resources
QT4_ADD_RESOURCES(RESOURCE_SRC main.qrc)

# Enable Qt-support
include(${QT_USE_FILE})

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} "-L${CMAKE_SOURCE_DIR}/src/launcherlib -lapplauncherd -L${CMAKE_SOURCE_DIR}/src/qdeclarativebooster ${booster_libs}")
add_executable(fala_qml ${SRC} ${RESOURCE_SRC})

# Install
install(PROGRAMS fala_qml DESTINATION /usr/bin/ RENAME fala_qml_wol)
install(PROGRAMS fala_qml DESTINATION /usr/bin/ RENAME fala_qml_helloworld)
install(PROGRAMS fala_qml DESTINATION /usr/bin/ RENAME fala_qml_wl)
install(PROGRAMS fala_qml DESTINATION /usr/share/fala_images RENAME fala_qml_helloworld)
install(FILES fala_qml_wl.desktop DESTINATION /usr/share/applications)
install(FILES fala_qml_wol.desktop DESTINATION /usr/share/applications)
install(SCRIPT scripts/create_links.cmake)
