set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Set sources
set(SRC ebooster.cpp pluginfactory.cpp)

# Set libraries to be linked.
link_libraries("-L../launcherlib -lapplauncherd" ${LIBDL})

# Set executable
add_library(ebooster MODULE ${SRC} ${MOC_SRC})
add_dependencies(ebooster applauncherd)

# Add install rule
install(TARGETS ebooster DESTINATION /usr/lib/applauncherd/)
