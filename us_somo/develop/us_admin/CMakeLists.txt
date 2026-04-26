add_executable(us3_admin
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries(us3_admin PRIVATE somo_generic)
target_include_directories(us3_admin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS us3_admin
        RUNTIME DESTINATION bin
        COMPONENT Programs
)