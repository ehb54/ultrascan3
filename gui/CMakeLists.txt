add_library(us_gui
    us_abstractrotor_gui.cpp us_abstractrotor_gui.h
    us_analysis_base2.cpp us_analysis_base2.h
    us_analyte_gui.cpp us_analyte_gui.h
    us_associations_gui.cpp us_associations_gui.h
    us_buffer_gui.cpp us_buffer_gui.h
    us_choice.cpp us_choice.h
    us_colorgradIO.cpp us_colorgradIO.h
    us_combined_plots_parms_gui.cpp us_combined_plots_parms_gui.h
    us_csv_loader.cpp us_csv_loader.h
    us_data_loader.cpp us_data_loader.h
    us_edit_spectrum.cpp us_edit_spectrum.h
    us_editor.cpp us_editor.h
    us_editor_gui.cpp us_editor_gui.h
    us_extinctfitter_gui.cpp us_extinctfitter_gui.h
    us_extinction_gui.cpp us_extinction_gui.h
    us_failed_gmp_run_gui.cpp us_failed_gmp_run_gui.h
    us_gui_settings.cpp us_gui_settings.h
    us_gui_util.cpp us_gui_util.h
    us_help.cpp us_help.h
    us_images.cpp us_images.h
    us_investigator.cpp us_investigator.h
    us_license.cpp us_license.h
    us_load_auc.cpp us_load_auc.h
    us_loadable_noise.cpp us_loadable_noise.h
    us_minimize.cpp us_minimize.h
    us_model_gui.cpp us_model_gui.h
    us_model_loader.cpp us_model_loader.h
    us_mwl_data.cpp us_mwl_data.h
    us_new_spectrum.cpp us_new_spectrum.h
    us_noise_loader.cpp us_noise_loader.h
    us_passwd.cpp us_passwd.h
    us_plot.cpp us_plot.h
    us_plot3d.cpp us_plot3d.h
    us_predict1.cpp us_predict1.h
    us_project_gui.cpp us_project_gui.h
    us_properties.cpp us_properties.h
    us_report_general_gui.cpp us_report_general_gui.h
    us_report_gui.cpp us_report_gui.h
    us_resids_bitmap.cpp us_resids_bitmap.h
    us_rotor_gui.cpp us_rotor_gui.h
    us_run_details2.cpp us_run_details2.h
    us_sassoc.cpp us_sassoc.h
    us_scan_excl_gui.cpp us_scan_excl_gui.h
    us_select_edits.cpp us_select_edits.h
    us_select_item.cpp us_select_item.h
    us_select_runs.cpp us_select_runs.h
    us_sim_params_gui.cpp us_sim_params_gui.h
    us_solution_gui.cpp us_solution_gui.h
    us_spectrodata.cpp us_spectrodata.h
    us_table.cpp us_table.h
    us_tmst_plot.cpp us_tmst_plot.h
    us_widgets.cpp us_widgets.h
    us_widgets_dialog.cpp us_widgets_dialog.h
    us_window_message.cpp us_window_message.h
)
# link against Qt
target_link_libraries(us_gui PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Xml
    Qt::Widgets
    Qt::Svg
    Qt::PrintSupport
    Qt::OpenGL
)
# link QWT
target_link_libraries(us_gui PUBLIC Qwt::Qwt)
# link threads
target_link_libraries(us_gui PUBLIC Threads::Threads)
# link against ultrascan libraries
target_link_libraries(us_gui PUBLIC qwtplot3d us_utils)

target_include_directories(us_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT "NO_DB" IN_LIST DEFINED)
    target_include_directories(us_gui PUBLIC /usr/include)
    target_include_directories(us_gui PUBLIC /usr/include/mysql)
    target_include_directories(us_gui PUBLIC "$ENV{MYSQLPATH}" "$ENV{MYSQLPATH}/include")
endif()

# Resources:
set(images_resource_files
    "images/analysis.png"
    "images/analysis_2.png"
    "images/arrow_left.png"
    "images/arrow_right.png"
    "images/bluearrow.png"
    "images/box.png"
    "images/check.png"
    "images/editing.png"
    "images/filecell.png"
    "images/fileopen.png"
    "images/filledmesh.png"
    "images/floordata.png"
    "images/floorempty.png"
    "images/flooriso.png"
    "images/floormesh.png"
    "images/frame.png"
    "images/greenarrow.png"
    "images/grid.png"
    "images/gridb.png"
    "images/gridc.png"
    "images/gridf.png"
    "images/gridfr.png"
    "images/gridl.png"
    "images/gridr.png"
    "images/hiddenline.png"
    "images/icon.png"
    "images/import_1.png"
    "images/import_2.png"
    "images/live_update.png"
    "images/movie.png"
    "images/new_item2.png"
    "images/nodata.png"
    "images/none.png"
    "images/normals.png"
    "images/polygon.png"
    "images/qwtplot.png"
    "images/redarrow.png"
    "images/report.png"
    "images/savecontent.png"
    "images/scattered.png"
    "images/setup.png"
    "images/table.png"
    "images/us3-icon-48x48.png"
    "images/us3-splash.png"
    "images/wireframe.png"
)

qt_add_resources(images_resource_files
        images.qrc
)
target_sources(us_gui PUBLIC ${images_resource_files})



if(UNIX)
    target_sources(us_gui PUBLIC
        us_x11_utils.c us_x11_utils.h
    )


    target_compile_definitions(us_gui PUBLIC
        INTEL
        LINUX
    )

    target_link_libraries(us_gui PUBLIC
        archive
        X11
        GLU
    )
endif()

if(WIN32)
    target_sources(us_gui PUBLIC
        us_win_utils.c us_win_utils.h
    )

    target_include_directories(us_gui PUBLIC
        ../gui
        ../qwtplot3d/include
        ../utils
        /include /include
        /src
        c:/msys64/usr/local/qwt/6.1.6/include
    )

    target_compile_definitions(us_gui PUBLIC
        INTEL
        QT_SHAREDMEMORY
        QT_SYSTEMSEMAPHORE
        QWT_DLL
        US_MAKE_DLL
        US_MAKE_GUI_DLL
        __LCC__
    )

    target_link_libraries(us_gui PUBLIC
        # Remove: L../lib
        # Remove: Lc:/msys64/usr/local/openssl_1.0.2d/lib
        # Remove: Lc:/msys64/usr/local/qwt/6.1.6/lib
        archive
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libadvapi32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libgdi32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libkernel32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libpsapi.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libuser32.a
        c:/msys64/mingw64/x86_64-w64-mingw32/lib/libws2_32.a
        crypto
        qwt
        qwtplot3d
        ssl
        us_utils10
    )
endif()

if(MACOS)
    target_include_directories(us_gui PUBLIC
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

    target_compile_definitions(us_gui PUBLIC
        MAC MAC
        OSX OSX
    )

    target_link_libraries(us_gui PUBLIC
        "-framework QtOpenGL"
        # Remove: L../lib/
        # Remove: L/System/Library/Frameworks/OpenGL.framework/Libraries
        # Remove: L/usr/X11R6/lib
        # Remove: L/usr/local/qwt-6.1.6/lib
        X11
        Xau
        archive
        crypto
        qwt
        qwtplot3d
        ssl
        us_utils
    )
endif()

if(APPLE)
    target_sources(us_gui PUBLIC
        us_mac_utils.c us_mac_utils.h
    )
endif()

#if(DEFINES___contains___NO_DB AND UNIX)
#    target_include_directories(us_gui PUBLIC
#        ../utils
#    )
#endif()
#
#if(UNIX AND NOT DEFINES___contains___NO_DB)
#    target_include_directories(us_gui PUBLIC
#        ../gui
#        ../utils
#        /include /include
#        /src
#    )
#endif()
#target_include_directories(us_gui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS us_gui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
