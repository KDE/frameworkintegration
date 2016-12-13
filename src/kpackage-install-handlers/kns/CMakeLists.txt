add_executable(knshandler main.cpp)
target_link_libraries(knshandler KF5::NewStuff)

install(TARGETS knshandler DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF5}/kpackagehandlers)
