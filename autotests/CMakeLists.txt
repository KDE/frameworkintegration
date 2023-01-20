include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt${QT_MAJOR_VERSION}::Test)
    message(STATUS "Qt${QT_MAJOR_VERSION}Test not found, autotests will not be built.")
    return()
endif()

include_directories( Qt${QT_MAJOR_VERSION}Gui_PRIVATE_INCLUDE_DIRS )

set(CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_kdeglobals")
configure_file(kdeplatformtheme_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeplatformtheme_config.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(FRAMEWORKINTEGRATION_TESTS _testname)
    ecm_add_test(${_testname}.cpp ${ARGN}
                 LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF6::ConfigWidgets KF6::ConfigCore KF6::IconThemes KF6::Style KF6::Notifications
                 TEST_NAME ${_testname}
                 NAME_PREFIX "frameworkintegration-")
    set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS "-DUNIT_TEST")
endmacro()

frameworkintegration_tests(
  kstyle_unittest
)

