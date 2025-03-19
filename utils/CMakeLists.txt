# Specify the library
add_library(utils SHARED
        us_ana_profile.cpp us_ana_profile.h
        us_analyte.cpp us_analyte.h
        us_archive.cpp us_archive.h
        us_astfem_math.cpp us_astfem_math.h
        us_astfem_rsa.cpp us_astfem_rsa.h
        us_buffer.cpp us_buffer.h
        us_cfa_data.cpp us_cfa_data.h
        us_constants.cpp us_constants.h
        us_crc.cpp us_crc.h
        us_csv_data.cpp us_csv_data.h
        us_dataIO.cpp us_dataIO.h
        us_datafiles.cpp us_datafiles.h
        us_db2.cpp us_db2.h
        us_dmga_constr.cpp us_dmga_constr.h
        us_eprofile.cpp us_eprofile.h
        us_geturl.cpp us_geturl.h
        us_global.cpp us_global.h
        us_gzip.cpp us_gzip.h
        us_hardware.cpp us_hardware.h
        us_http_post.cpp us_http_post.h
        us_hydrosim.cpp us_hydrosim.h
        us_lamm_astfvm.cpp us_lamm_astfvm.h
        us_link_ssl.cpp us_link_ssl.h
        us_lm.cpp us_lm.h
        us_local_server.cpp us_local_server.h
        us_math2.cpp us_math2.h
        us_matrix.cpp us_matrix.h
        us_memory.cpp us_memory.h
        us_model.cpp us_model.h
        us_noise.cpp us_noise.h
        us_pcsa_modelrec.cpp us_pcsa_modelrec.h
        us_project.cpp us_project.h
        us_protocol_util.cpp us_protocol_util.h
        us_report.cpp us_report.h
        us_report_gmp.cpp us_report_gmp.h
        us_rotor.cpp us_rotor.h
        us_run_protocol.cpp us_run_protocol.h
        us_settings.cpp us_settings.h
        us_simparms.cpp us_simparms.h
        us_solute.cpp us_solute.h
        us_solution.cpp us_solution.h
        us_solution_vals.cpp us_solution_vals.h
        us_solve_sim.cpp us_solve_sim.h
        us_stiffbase.cpp us_stiffbase.h
        us_tar.cpp us_tar.h
        us_time_state.cpp us_time_state.h
        us_timer.cpp us_timer.h
        us_util.cpp us_util.h
        us_vector.cpp us_vector.h
        us_xpn_data.cpp us_xpn_data.h
        us_zsolute.cpp us_zsolute.h
        )

# Ensure debug information is included
target_compile_options(utils PRIVATE -g -O0)

# Link Qt libraries to the utils library
target_link_libraries(utils Qt5::Core Qt5::Sql Qt5::Network Qt5::Xml)

target_link_libraries(utils archive)

# Specify include directories for utils
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(utils PUBLIC /usr/include)
target_include_directories(utils PUBLIC /usr/include/mysql)