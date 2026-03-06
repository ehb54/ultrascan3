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

execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${DOC_QCH_STAGE}" "${DOC_OUTPUT_DIR}/manual.qch")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${DOC_QHC_STAGE}" "${DOC_OUTPUT_DIR}/manual.qhc")
execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${ASSISTANT_SOURCE}" "${DOC_OUTPUT_DIR}/Assistant.exe")

