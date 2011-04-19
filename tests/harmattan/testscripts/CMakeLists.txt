if ($ENV{MEEGO})
    # Do nothing
endif ($ENV{MEEGO})

if ($ENV{HARMATTAN})
    install(FILES
        test-perf-mbooster.py
        check_pipes.py
        test-func-launcher.py
        utils.py
        test-security.py
        test-daemons.py 
        test-single-instance.py
        test-core-dump.py
        DESTINATION /usr/share/applauncherd-testscripts)

    install(PROGRAMS
        ts_prestartapp.rb
        tc_theming.rb
        tc_splash.rb
        get-coordinates.rb
        test-perf.rb
        fala_xres_wl
        fala_xres_wol
        DESTINATION /usr/share/applauncherd-testscripts)

    install(PROGRAMS fala_wid DESTINATION /usr/bin/)

    add_subdirectory (signal-forward)
endif ($ENV{HARMATTAN})
