# Set sources
set(SRC main.cpp)

execute_process(COMMAND "env" 
                         "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                         "/usr/bin/pkg-config" 
                         "--cflags" 
                         "qt-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_cflags 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "env" 
                        "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                        "/usr/bin/pkg-config" "--libs" 
                        "qt-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_libs 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${booster_cflags}")
set(CMAKE_EXE_LINKER_FLAGS ${booster_libs})

# Enable Qt-support
include(${QT_USE_FILE})
link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
add_executable(fala_testapp ${SRC})

# Install
install(PROGRAMS fala_testapp DESTINATION /usr/bin RENAME fala_wol)
install(PROGRAMS fala_testapp DESTINATION /usr/bin RENAME fala_wl)
install(FILES com.nokia.fala_testapp.service com.nokia.fala_wl.service com.nokia.fala_wol.service DESTINATION /usr/share/dbus-1/services)
install(FILES fala_wl.desktop fala_wol.desktop DESTINATION /usr/share/applications/)
install(FILES images/landscape.jpg DESTINATION /usr/share/fala_images/)
install(FILES images/portrait.jpg DESTINATION /usr/share/fala_images/)

