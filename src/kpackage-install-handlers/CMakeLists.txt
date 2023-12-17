add_subdirectory(kns)

if(AppStreamQt5_FOUND AND packagekitqt5_FOUND)
    add_subdirectory(appstream)
endif()
