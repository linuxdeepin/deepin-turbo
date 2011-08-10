if ($ENV{MEEGO})
    # Do nothing
endif ($ENV{MEEGO})

if ($ENV{HARMATTAN})
    install(FILES
        test-func-launcher.py
        utils.py
        test-security.py
        test-invoker.py
        test-daemons.py 
        test-single-instance.py
        test-boot-mode.py
        DESTINATION /usr/share/applauncherd-testscripts)

    install(PROGRAMS
        tc_splash.rb
        get-coordinates.rb
        test-perf.rb
        DESTINATION /usr/share/applauncherd-testscripts)

    install(PROGRAMS fala_wid DESTINATION /usr/bin/)

    add_subdirectory (signal-forward)
endif ($ENV{HARMATTAN})
