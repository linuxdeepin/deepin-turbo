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
  test-perf.rb
  fala_wid
  fala_xres_wl
  fala_xres_wol
  DESTINATION /usr/share/applauncherd-testscripts)

add_subdirectory (signal-forward)
