cmake_minimum_required(VERSION 2.8)
find_package(Qt4 4.7.0 REQUIRED)
include(${QT_USE_FILE})
include(FindPkgConfig)
pkg_check_modules(QDECLARATIVE_BOOSTABLE REQUIRED qdeclarative-boostable)
add_definitions(${QDECLARATIVE_BOOSTABLE_CFLAGS})
link_libraries(${QDECLARATIVE_BOOSTABLE_LDFLAGS} ${QT_QTCORE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})
add_executable(qml-helloworld main.cpp)

