add_executable(us_buffer WIN32 MACOSX_BUNDLE
    ../../utils/us_buffer.h
    us_buffer_main.cpp
)
target_link_libraries(us_buffer PUBLIC us_common)

install(TARGETS us_buffer
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
