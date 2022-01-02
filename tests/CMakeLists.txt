add_executable(kstyletest kstyletest.cpp)
target_link_libraries(kstyletest Qt${QT_MAJOR_VERSION}::Widgets KF5::Style)

