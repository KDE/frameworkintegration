include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMAddTests)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

include_directories( ${Qt5Gui_PRIVATE_INCLUDE_DIRS} )

set(CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_kdeglobals")
configure_file(kdeplatformtheme_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeplatformtheme_config.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

macro(FRAMEWORKINTEGRATION_TESTS _testname)
    ecm_add_test(${_testname}.cpp ${ARGN}
                 LINK_LIBRARIES Qt5::Test KF5::ConfigWidgets KF5::ConfigCore KF5::IconThemes KF5::Style KF5::Notifications
                 TEST_NAME ${_testname}
                 NAME_PREFIX "frameworkintegration-")
    set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS "-DUNIT_TEST")
endmacro()

frameworkintegration_tests(
  kstyle_unittest
)

