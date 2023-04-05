# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Style")

add_library(KF6Style kstyle.cpp)
add_library(KF6::Style ALIAS KF6Style)

set_target_properties(KF6Style PROPERTIES
    VERSION     ${FRAMEWORKINTEGRATION_VERSION}
    SOVERSION   ${FRAMEWORKINTEGRATION_SOVERSION}
    EXPORT_NAME Style
)

generate_export_header(KF6Style BASE_NAME KStyle)

target_include_directories(KF6Style
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/FrameworkIntegration>;$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KStyle>")

target_link_libraries(KF6Style
    PUBLIC
        Qt6::Widgets
    PRIVATE
        KF6::ConfigWidgets
        KF6::IconThemes
)

ecm_generate_headers(KStyle_HEADERS
  HEADER_NAMES
    KStyle
  REQUIRED_HEADERS KStyle_HEADERS
)

install(TARGETS KF6Style
        EXPORT KF6FrameworkIntegrationTargets
        ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kstyle_export.h
              ${KStyle_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KStyle
        COMPONENT Devel)

