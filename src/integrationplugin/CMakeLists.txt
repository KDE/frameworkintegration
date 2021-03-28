
add_library(FrameworkIntegrationPlugin
    MODULE frameworkintegrationplugin.cpp)

target_link_libraries(FrameworkIntegrationPlugin
    PRIVATE
        KF5::WidgetsAddons
        KF5::ConfigCore
        Canberra::Canberra
)

install(TARGETS FrameworkIntegrationPlugin
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5)
