# Validate the final staged help payload, after platform deployment.
set(_required_help "${S_BIN}/manual.qch" "${S_BIN}/manual.qhc")
if(WIN32)
    list(APPEND _required_help "${S_BIN}/assistant.exe"
        "${S_BIN}/sqlite3.dll" "${S_PLUG}/sqldrivers/qsqlite.dll")
elseif(APPLE)
    list(APPEND _required_help "${S_BIN}/Assistant.app/Contents/MacOS/Assistant"
        "${S_PLUG}/sqldrivers/libqsqlite.dylib")
else()
    list(APPEND _required_help "${S_BIN}/assistant"
        "${S_PLUG}/sqldrivers/libqsqlite.so")
endif()
foreach(_file IN LISTS _required_help)
    if(NOT EXISTS "${_file}" OR IS_DIRECTORY "${_file}")
        message(FATAL_ERROR "Required staged help component is missing: ${_file}")
    endif()
    file(SIZE "${_file}" _size)
    if(_size EQUAL 0)
        message(FATAL_ERROR "Required staged help component is empty: ${_file}")
    endif()
endforeach()
message(STATUS "Verified staged APP help components")
