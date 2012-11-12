# Set sources
set(SRC fala_wait.cpp)

include(${QT_USE_FILE})

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

link_libraries(${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
add_executable(fala_wait ${SRC})

# Install
install(PROGRAMS fala_wait DESTINATION /usr/bin/ )

