set(SRC creds_test.cpp)

link_libraries(${MEEGOTOUCH_LIBRARIES})

include(${QT_USE_FILE})
add_library(creds_test MODULE ${SRC})

# max creds
install(PROGRAMS libcreds_test.so DESTINATION /usr/bin RENAME fala_ft_creds1.launch)
install(PROGRAMS fala_ft_creds DESTINATION /usr/bin RENAME fala_ft_creds1)

# "normal" creds
install(PROGRAMS libcreds_test.so DESTINATION /usr/bin RENAME fala_ft_creds2.launch)
install(PROGRAMS fala_ft_creds DESTINATION /usr/bin RENAME fala_ft_creds2)

