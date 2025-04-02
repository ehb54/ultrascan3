add_executable(us_analyte_gui WIN32 MACOSX_BUNDLE
    us_analyte_main.cpp
)
target_link_libraries(us_analyte_gui PUBLIC us_common)

install(TARGETS us_analyte_gui
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
