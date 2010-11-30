if ($ENV{HARMATTAN})
    install(FILES tests.xml DESTINATION /usr/share/applauncherd-performance-tests/)
endif ($ENV{HARMATTAN})

if ($ENV{MEEGO})
    # Do nothing
endif ($ENV{MEEGO})
