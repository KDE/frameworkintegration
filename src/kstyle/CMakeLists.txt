
find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
    DESCRIPTION "X protocol C-language Binding"
    URL "http://xcb.freedesktop.org"
    TYPE OPTIONAL
    PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

if(NOT APPLE)
  set(HAVE_X11 ${XCB_XCB_FOUND})
  if (XCB_XCB_FOUND)
    find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED X11Extras)
  endif()
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Style")

configure_file(config-kstyle.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstyle.h )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

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

if(HAVE_X11)
  target_link_libraries(KF5Style PRIVATE Qt5::X11Extras ${XCB_XCB_LIBRARY})
endif()

set_target_properties(KF5Style PROPERTIES VERSION   ${FRAMEWORKINTEGRATION_VERSION_STRING}
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

