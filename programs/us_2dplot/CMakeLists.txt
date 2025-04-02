add_executable(us_2dplot WIN32 MACOSX_BUNDLE
    us_2dplot.cpp us_2dplot.h
)


target_link_libraries(us_2dplot PUBLIC us_common)

install(TARGETS us_2dplot
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

