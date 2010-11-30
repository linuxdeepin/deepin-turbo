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
        DESTINATION /usr/share/applauncherd-testscripts)

    install(PROGRAMS
        ts_prestartapp.rb
        tc_theming.rb
        get-coordinates.rb
        test-perf.rb
        fala_wid
        fala_xres_wl
        fala_xres_wol
        DESTINATION /usr/share/applauncherd-testscripts)

    add_subdirectory (signal-forward)
endif ($ENV{HARMATTAN})
