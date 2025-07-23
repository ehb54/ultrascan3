add_library(us_utils
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
    us_license_t.cpp us_license_t.h
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
target_include_directories(us_utils PUBLIC ${CMAKE_SOURCE_DIR})
# Link Against Qt5 libraries
target_link_libraries(us_utils PUBLIC
    Qt::Core
    Qt::Network
    Qt::Sql
    Qt::Xml
)

# linking against threads
target_link_libraries(us_utils PUBLIC Threads::Threads)

if(NOT "NO_DB" IN_LIST DEFINED)
    target_sources(us_utils PUBLIC
            us_crypto.cpp us_crypto.h
    )
    target_include_directories(us_utils PUBLIC /usr/include)
    target_include_directories(us_utils PUBLIC /usr/include/mysql)
    target_include_directories(us_utils PUBLIC "$ENV{MYSQLPATH}" "$ENV{MYSQLPATH}/include")
    target_link_libraries(us_utils PUBLIC mysqlclient ssl crypto)
endif()


if(UNIX)
    #target_include_directories(us_utils PUBLIC
    #    /src
    #)

    target_compile_definitions(us_utils PUBLIC
        INTEL
        LINUX
        QT_DISABLE_DEPRECATED_UP_TO=0x050F00
    )
endif()

if(MACOS)
    target_include_directories(us_utils PUBLIC
        ../../Frameworks/QtCore.framework/Headers
        ../../Frameworks/QtGui.framework/Headers
        ../../Frameworks/QtOpenGL.framework/Headers
        ../../Frameworks/QtSvg.framework/Headers
        ../../Frameworks/QtXml.framework/Headers
        ../../qwtplot3d/include
        ../Frameworks/QtCore.framework/Headers
        ../Frameworks/QtGui.framework/Headers
        ../Frameworks/QtOpenGL.framework/Headers
        ../Frameworks/QtSvg.framework/Headers
        ../Frameworks/QtXml.framework/Headers
        ../gui
        ../mysql/include
        ../qwtplot3d/include
        ../utils
        /include
        /usr/X11R6/include
    )

    target_compile_definitions(us_utils PUBLIC
        MAC MAC
        OSX OSX
    )

    target_link_libraries(us_utils PUBLIC
        "-framework QtOpenGL"
        # Remove: L../lib
        # Remove: L/System/Library/Frameworks/OpenGL.framework/Libraries
        # Remove: L/usr/local/mysql/lib
        # Remove: L/usr/local/qwt-6.1.6/lib
        archive
        crypto
        mysqlclient
        qwt
        qwtplot3d
        ssl
    )
endif()

if(WIN32)
    target_include_directories(us_utils PUBLIC
        ../gui
        ../qwtplot3d/include
        ../utils
        /include /include
        /src
        $ENV{QWTPATH}/include
        $ENV{QWTINC}
    )

    target_compile_definitions(us_utils PUBLIC
        INTEL
        QT_SHAREDMEMORY
        QT_SYSTEMSEMAPHORE
        US_MAKE_DLL
        US_MAKE_UTIL_DLL
        __LCC__
    )

    target_link_libraries(us_utils PUBLIC
        # Remove: Lc:/msys64/usr/local/openssl_1.0.2d/lib
        archive
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libadvapi32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libgdi32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libkernel32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libpsapi.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libuser32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libws2_32.a
        gdi32

    )
endif()

#if(NOT DEFINES___contains___NO_DB)
#    target_sources(us_utils PUBLIC
#        us_crypto.cpp us_crypto.h
#    )
#endif()
#
#if(DEFINES___contains___NO_DB)
#    target_link_libraries(us_utils PUBLIC
#        # Remove: L/lib
#    )
#endif()
#
#if(DEFINES___contains___NO_DB AND UNIX)
#    target_include_directories(us_utils PUBLIC
#        ../utils
#    )
#endif()
#
#if(UNIX AND NOT DEFINES___contains___NO_DB)
#    target_include_directories(us_utils PUBLIC
#        ../gui
#        ../utils
#        /include /include
#        /src
#    )
#endif()

install(TARGETS us_utils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
