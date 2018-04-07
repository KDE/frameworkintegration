cmake_minimum_required(VERSION 3.0)

set(KF5_VERSION "5.45.0") # handled by release scripts
set(KF5_DEP_VERSION "5.45.0") # handled by release scripts
project(FrameworkIntegration VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.45.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})


include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.8.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus Widgets)


ecm_setup_version(PROJECT VARIABLE_PREFIX FRAMEWORKINTEGRATION
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5FrameworkIntegrationConfigVersion.cmake"
                  SOVERSION 5)

find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Notifications ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Package ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5NewStuff ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)

find_package(packagekitqt5)
find_package(AppStreamQt 0.10.4)

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5FrameworkIntegration")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5FrameworkIntegrationConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5FrameworkIntegrationConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5FrameworkIntegrationConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5FrameworkIntegrationConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5FrameworkIntegrationTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5FrameworkIntegrationTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )

install(FILES plasma_workspace.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})


feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
