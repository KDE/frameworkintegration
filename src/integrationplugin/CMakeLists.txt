
add_library(FrameworkIntegrationPlugin
    MODULE frameworkintegrationplugin.cpp)

target_link_libraries(FrameworkIntegrationPlugin
    PRIVATE
        KF5::WidgetsAddons
        KF5::ConfigCore
        KF5::Notifications
)

install(TARGETS FrameworkIntegrationPlugin
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION})
