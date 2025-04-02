add_executable(us_astfem_sim WIN32 MACOSX_BUNDLE
    us_astfem_sim.cpp us_astfem_sim.h
    us_clipdata.cpp us_clipdata.h
)
target_link_libraries(us_astfem_sim PUBLIC us_common)

install(TARGETS us_astfem_sim
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)