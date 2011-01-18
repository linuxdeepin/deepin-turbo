# Set sources
set(SRC fala_multi-instance.cpp mainpage.cpp mainwindow.cpp)
set(MOC_HDRS mainpage.h mainwindow.h)
qt4_wrap_cpp(MOC_SRC ${MOC_HDRS})

link_libraries(${MEEGOTOUCH_LIBRARIES})

# Use the compiler and linker flags given in meegotouch-boostable.pc
# in the source tree.
execute_process(COMMAND "env" 
                         "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                         "/usr/bin/pkg-config" 
                         "--cflags" 
                         "meegotouch-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_cxxflags 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "env" 
                        "PKG_CONFIG_PATH=${CMAKE_SOURCE_DIR}/data/pkgconfig" 
                        "/usr/bin/pkg-config" "--libs" 
                        "meegotouch-boostable" 
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
                OUTPUT_VARIABLE booster_libs 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${booster_cxxflags}")
set(CMAKE_EXE_LINKER_FLAGS ${booster_libs})

# Enable Qt-support
include(${QT_USE_FILE})

add_executable(fala_multi-instance ${SRC} ${MOC_SRC})

# Install
install(PROGRAMS fala_multi-instance DESTINATION /usr/bin/)

