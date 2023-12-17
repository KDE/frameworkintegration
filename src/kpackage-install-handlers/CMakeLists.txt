add_subdirectory(kns)

if(AppStreamQt_FOUND AND packagekitqt5_FOUND)
    add_subdirectory(appstream)
endif()
