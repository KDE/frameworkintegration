add_executable(knshandler main.cpp)
target_link_libraries(knshandler KF5::NewStuffCore KF5::I18n KF5::Notifications)

install(TARGETS knshandler DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF5}/kpackagehandlers)

add_executable(knshandlertest main.cpp)
target_link_libraries(knshandlertest KF5::NewStuffCore KF5::I18n KF5::Notifications)
target_compile_definitions(knshandlertest PRIVATE -DTEST)

if(EXISTS "${CMAKE_INSTALL_PREFIX}/${CONFIG_INSTALL_DIR}/colorschemes.knsrc")
    add_test(NAME test_kns-kpackage COMMAND knshandlertest kns://colorschemes.knsrc/api.kde-look.org/1136471)

    add_test(NAME test_kns-kpackage-fail COMMAND knshandlertest kns://colorschemes.knsrc/xxx/1136471)
    set_tests_properties(test_kns-kpackage-fail PROPERTIES WILL_FAIL TRUE)
    message(STATUS "KNS-KPackage test enabled")
endif()
