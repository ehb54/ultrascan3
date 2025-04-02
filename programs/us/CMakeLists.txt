add_executable(us
    us.cpp us.h
    us_revision.h
    us_win_data.cpp
)

target_link_libraries(us PUBLIC us_common)

install(TARGETS us
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

