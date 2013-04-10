# Find Qt4
find_package(Qt4 4.6.0 REQUIRED)
include(${QT_USE_FILE})
if (${QTVERSION} VERSION_LESS 4.6.0)
    message(FATAL_ERROR "You need Qt4.6, found ${QTVERSION}.")
endif (${QTVERSION} VERSION_LESS 4.6.0)

add_subdirectory(ut_booster)
add_subdirectory(ut_connection)
add_subdirectory(ut_daemon)
add_subdirectory(ut_ebooster)
add_subdirectory(ut_socketmanager)
add_subdirectory(ut_singleinstance)
add_subdirectory(ut_boosterpluginregistry)
add_subdirectory(ut_appdata)
add_subdirectory(ut_logger)

install(FILES tests.xml DESTINATION /usr/share/applauncherd-tests)

