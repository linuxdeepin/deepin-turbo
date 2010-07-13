install(FILES
  test-perf-mbooster.py
  check_pipes.py
  test-func-launcher.py
  DESTINATION /usr/share/applauncherd-testscripts)

install(PROGRAMS
  ts_prestartapp.rb
  tc_theming.rb
  DESTINATION /usr/share/applauncherd-testscripts)
