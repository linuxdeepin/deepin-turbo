execute_process(COMMAND echo "Creating symlinks for fala_ft_hello..")
execute_process(COMMAND ln -v -s /usr/bin/fala_ft_hello $ENV{DESTDIR}/usr/bin/fala_ft_hello1)
execute_process(COMMAND ln -v -s /usr/bin/fala_e_helloworld $ENV{DESTDIR}/usr/bin/fala_e_helloworld1)
execute_process(COMMAND ln -v -s /usr/bin/fala_e_helloworld $ENV{DESTDIR}/usr/bin/fala_e_helloworld2)
execute_process(COMMAND ln -v -s /usr/bin/fala_ft_hello $ENV{DESTDIR}/usr/bin/fala_ft_hello2)

