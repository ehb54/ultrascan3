us3_add_gui_test(test_us_resplot_fem
    SOURCES
        test_us_resplot_fem.cpp
        legacy_dialog.cpp
        ${CMAKE_SOURCE_DIR}/programs/us_fematch/us_resplot_fem.h
    LIBRARIES UltraScan3::Gui_Static Qt::DataVisualization
    INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/programs/us_fematch
    LABELS programs;residual-characterization
)
# Keep real application declarations out of the executable containing doubles:
# signature checks must compile, but their object is deliberately never linked.
add_library(test_residual_owner_contract OBJECT owner_contract.cpp)
target_link_libraries(test_residual_owner_contract PRIVATE UltraScan3::Gui_Static Qt::DataVisualization)
set_target_properties(test_residual_owner_contract PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
add_dependencies(test_us_resplot_fem test_residual_owner_contract)
set_tests_properties(test_us_resplot_fem PROPERTIES TIMEOUT 120)
# Repeated lifecycle cases otherwise exhaust QtTest's message budget on qDebug.
# Keep warnings enabled so unexpected Qt lifetime warnings remain visible.
set_property(TEST test_us_resplot_fem APPEND PROPERTY ENVIRONMENT "QT_LOGGING_RULES=*.debug=false")
