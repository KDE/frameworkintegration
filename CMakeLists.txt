cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.91.0") # handled by release scripts
set(KF_DEP_VERSION "5.90.0") # handled by release scripts
project(FrameworkIntegration VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 5.90.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

set(REQUIRED_QT_VERSION 5.15.2)
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui Widgets)


ecm_setup_version(PROJECT VARIABLE_PREFIX FRAMEWORKINTEGRATION
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/frameworkintegration_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5FrameworkIntegrationConfigVersion.cmake"
                  SOVERSION 5)

find_package(KF5Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Notifications ${KF_DEP_VERSION} REQUIRED)


option(BUILD_KPACKAGE_INSTALL_HANDLERS "Build the KPackage install handler binaries (recommended)" ON)
if (BUILD_KPACKAGE_INSTALL_HANDLERS)
   find_package(KF5NewStuff ${KF_DEP_VERSION} REQUIRED)
   find_package(KF5WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
   find_package(KF5Package ${KF_DEP_VERSION} REQUIRED)
   find_package(KF5I18n ${KF_DEP_VERSION} REQUIRED)

   find_package(packagekitqt${QT_MAJOR_VERSION})
   find_package(AppStreamQt 0.10.4)
endif()
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f02)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

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
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/FrameworkIntegration COMPONENT Devel)

install(FILES plasma_workspace.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})


feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
