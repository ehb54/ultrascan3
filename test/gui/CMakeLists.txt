us3_add_gui_test(test_us_editor_gui
        SOURCES
            test_us_editor_gui.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        LABELS
            gui
        )

us3_add_gui_test(test_us_gui_util_svg
        SOURCES
            test_us_gui_util_svg.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        LABELS
            gui
        )

us3_add_gui_test(test_us_gui_test_isolation
        SOURCES
            test_us_gui_test_isolation.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        LABELS
            gui
        )

us3_add_gui_test(test_us_widgets_counter
        SOURCES
            test_us_widgets_counter.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        LABELS
            gui
        )

us3_add_gui_test(test_us_passwd_gui
        SOURCES
            test_us_passwd_gui.cpp
        LIBRARIES
            UltraScan3::Gui_Static
        LABELS
            gui
        )
