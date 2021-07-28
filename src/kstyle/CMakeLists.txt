# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Style")

add_library(KF5Style kstyle.cpp)
generate_export_header(KF5Style BASE_NAME KStyle)
add_library(KF5::Style ALIAS KF5Style)

target_include_directories(KF5Style INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KStyle>")

target_link_libraries(KF5Style
    PUBLIC
        Qt5::Widgets
    PRIVATE
        KF5::ConfigWidgets
        KF5::IconThemes
)

set_target_properties(KF5Style PROPERTIES VERSION   ${FRAMEWORKINTEGRATION_VERSION}
                                          SOVERSION ${FRAMEWORKINTEGRATION_SOVERSION}
                                          EXPORT_NAME Style
)

ecm_generate_headers(KStyle_HEADERS
  HEADER_NAMES
    KStyle
  REQUIRED_HEADERS KStyle_HEADERS
)

install(TARGETS KF5Style
        EXPORT KF5FrameworkIntegrationTargets
        ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kstyle_export.h
              ${KStyle_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KStyle
        COMPONENT Devel)

