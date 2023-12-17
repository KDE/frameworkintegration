add_executable(appstreamhandler main.cpp)
target_link_libraries(appstreamhandler PK::packagekitqt5 AppStreamQt5)
install(TARGETS appstreamhandler DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF}/kpackagehandlers)
