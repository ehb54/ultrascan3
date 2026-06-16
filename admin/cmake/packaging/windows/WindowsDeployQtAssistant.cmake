string(REGEX REPLACE "^\"|\"$" "" DOC_OUTPUT_DIR "${DOC_OUTPUT_DIR}")
string(REGEX REPLACE "^\"|\"$" "" DOC_QCH_STAGE "${DOC_QCH_STAGE}")
string(REGEX REPLACE "^\"|\"$" "" DOC_QHC_STAGE "${DOC_QHC_STAGE}")
string(REGEX REPLACE "^\"|\"$" "" ASSISTANT_SOURCE "${ASSISTANT_SOURCE}")
string(REGEX REPLACE "^\"|\"$" "" WINDEPLOYQT "${WINDEPLOYQT}")
string(REGEX REPLACE "^\"|\"$" "" VCPKG_RUNTIME_BIN_DIR "${VCPKG_RUNTIME_BIN_DIR}")

file(MAKE_DIRECTORY "${DOC_OUTPUT_DIR}")

if(NOT EXISTS "${DOC_QCH_STAGE}")
    message(FATAL_ERROR "Missing Qt help file: ${DOC_QCH_STAGE}")
endif()

if(NOT EXISTS "${DOC_QHC_STAGE}")
    message(FATAL_ERROR "Missing Qt help collection: ${DOC_QHC_STAGE}")
endif()

if(NOT EXISTS "${ASSISTANT_SOURCE}")
    message(FATAL_ERROR "Missing Assistant.exe: ${ASSISTANT_SOURCE}")
endif()

set(STAGED_ASSISTANT "${DOC_OUTPUT_DIR}/Assistant.exe")

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${DOC_QCH_STAGE}" "${DOC_OUTPUT_DIR}/manual.qch"
        RESULT_VARIABLE _copy_qch_result
)
if(NOT _copy_qch_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy manual.qch")
endif()

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${DOC_QHC_STAGE}" "${DOC_OUTPUT_DIR}/manual.qhc"
        RESULT_VARIABLE _copy_qhc_result
)
if(NOT _copy_qhc_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy manual.qhc")
endif()

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${ASSISTANT_SOURCE}" "${STAGED_ASSISTANT}"
        RESULT_VARIABLE _copy_assistant_result
)
if(NOT _copy_assistant_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy Assistant.exe")
endif()

if(WINDEPLOYQT AND EXISTS "${WINDEPLOYQT}")
    execute_process(
            COMMAND "${WINDEPLOYQT}" --no-translations --no-system-d3d-compiler "${STAGED_ASSISTANT}"
            RESULT_VARIABLE _deploy_result
    )
    if(NOT _deploy_result EQUAL 0)
        message(FATAL_ERROR "windeployqt failed for Assistant.exe with code ${_deploy_result}")
    endif()
else()
    message(FATAL_ERROR "WINDEPLOYQT was not provided to WindowsDeployQtAssistant.cmake")
endif()

# Critical for Qt Help on Windows:
# qsqlite.dll depends on sqlite3.dll, and windeployqt does not always copy it.
if(VCPKG_RUNTIME_BIN_DIR AND EXISTS "${VCPKG_RUNTIME_BIN_DIR}/sqlite3.dll")
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${VCPKG_RUNTIME_BIN_DIR}/sqlite3.dll"
            "${DOC_OUTPUT_DIR}/sqlite3.dll"
            RESULT_VARIABLE _copy_sqlite_result
    )
    if(NOT _copy_sqlite_result EQUAL 0)
        message(FATAL_ERROR "Failed to copy sqlite3.dll")
    endif()
else()
    message(WARNING "sqlite3.dll not found in VCPKG_RUNTIME_BIN_DIR='${VCPKG_RUNTIME_BIN_DIR}'")
endif()
