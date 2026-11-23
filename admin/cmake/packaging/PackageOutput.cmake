# A successful package target records its exact output for wrappers/CI.
include_guard(GLOBAL)
function(us3_record_package_output target package_path)
    set(_receipt "${CMAKE_BINARY_DIR}/package-path.txt")
    add_custom_command(TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -f "${_receipt}"
        VERBATIM)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            "-DPACKAGE_PATH=${package_path}"
            "-DRECEIPT_FILE=${_receipt}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/WritePackageReceipt.cmake"
        VERBATIM)
endfunction()
