add_executable(us_2dsa WIN32 MACOSX_BUNDLE
    us_2dsa.cpp us_2dsa.h
    us_2dsa_process.cpp us_2dsa_process.h
    us_adv_analysis_2d.cpp us_adv_analysis_2d.h
    us_analysis_control_2d.cpp us_analysis_control_2d.h
    us_plot_control_2d.cpp us_plot_control_2d.h
    us_resplot_2d.cpp us_resplot_2d.h
    us_show_norm.cpp us_show_norm.h
    us_worker_2d.cpp us_worker_2d.h
    us_worker_calcnorm.cpp us_worker_calcnorm.h
)
target_link_libraries(us_2dsa PUBLIC us_common)

install(TARGETS us_2dsa
        FRAMEWORK DESTINATION   ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        RUNTIME DESTINATION   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

