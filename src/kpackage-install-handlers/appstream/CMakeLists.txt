add_executable(appstreamhandler main.cpp)
target_link_libraries(appstreamhandler PK::packagekitqt5 AppStreamQt)
install(TARGETS appstreamhandler DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF}/kpackagehandlers)
