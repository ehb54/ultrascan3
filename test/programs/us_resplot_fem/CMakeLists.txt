us3_add_gui_test(test_us_resplot_fem
    SOURCES
        test_us_resplot_fem.cpp
    LIBRARIES UltraScan3::Gui_Static Qt::DataVisualization
    LABELS programs
)
set_tests_properties(test_us_resplot_fem PROPERTIES TIMEOUT 120)
# Repeated lifecycle cases otherwise exhaust QtTest's message budget on qDebug.
# Keep warnings enabled so unexpected Qt lifetime warnings remain visible.
set_property(TEST test_us_resplot_fem APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
