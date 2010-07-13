# Find libmeegotouch for the test programs
include(FindPkgConfig) 
pkg_check_modules(MEEGOTOUCH meegotouch>=0.20 REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MEEGOTOUCH_INCLUDE_DIRS})

# Check if it's possible to compile MBooster with MComponentCache. If so,
# HAVE_MCOMPONENTCACHE is set.
message(STATUS "checking for mcomponentcache.h")
find_file(M_COMPONENT_CACHE NAMES mcomponentcache.h PATHS ${MEEGOTOUCH_INCLUDE_DIRS})
if (EXISTS ${M_COMPONENT_CACHE})
  message(STATUS "  found")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_MCOMPONENTCACHE")
else (EXISTS ${M_COMPONENT_CACHE})
  message(STATUS "  not found")
endif (EXISTS ${M_COMPONENT_CACHE})

# Sub build: helloworld
add_subdirectory(TestApps/helloworld)

# Sub build: unit tests
add_subdirectory(unittests)

# Sub build: perftests
add_subdirectory(perftests)

# Sub build: functests
add_subdirectory(functests)

# Sub build: art-tests 
add_subdirectory(art-tests)

# Sub build: bug-tests 
add_subdirectory(bug-tests)

# Sub build: TestScripts
add_subdirectory(TestScripts)

# Sub build: TestApps
add_subdirectory(TestApps/creds)

# Sub build: TestApps
add_subdirectory(TestApps/testapp)

# Sub build: TestApps
add_subdirectory(TestApps/themetest)
