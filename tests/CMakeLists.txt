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

# Sub build: TestApps
add_subdirectory(TestApps/fala_status)

# Sub build: TestApps
add_subdirectory(TestApps/fala_gettime)
