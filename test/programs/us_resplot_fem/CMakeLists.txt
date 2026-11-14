us3_add_gui_test(test_us_resplot_fem
    SOURCES
        test_us_resplot_fem.cpp
        ${CMAKE_SOURCE_DIR}/programs/us_fematch/us_resplot_fem.cpp
        ${CMAKE_SOURCE_DIR}/programs/us_fematch/us_resplot_fem.h
    LIBRARIES UltraScan3::Gui_Static Qt::DataVisualization
    INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/programs/us_fematch
    LABELS programs;residual-characterization
)
set_tests_properties(test_us_resplot_fem PROPERTIES TIMEOUT 120)
# Repeated lifecycle cases otherwise exhaust QtTest's message budget on qDebug.
# Keep warnings enabled so unexpected Qt lifetime warnings remain visible.
set_property(TEST test_us_resplot_fem APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
