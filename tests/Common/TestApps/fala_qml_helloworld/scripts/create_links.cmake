execute_process(COMMAND echo "Creating symlinks for fala_qml_helloworld..")
execute_process(COMMAND ln -v -s /usr/bin/fala_qml_helloworld $ENV{DESTDIR}/usr/bin/fala_qml_helloworld1)
execute_process(COMMAND ln -v -s /usr/bin/fala_qml_helloworld $ENV{DESTDIR}/usr/bin/fala_qml_helloworld2)

