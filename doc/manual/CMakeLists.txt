# UltraScan III Documentation Build
cmake_minimum_required(VERSION 3.10)

# Find required tools
find_program(QHELPGENERATOR_EXECUTABLE NAMES qhelpgenerator
        HINTS ${Qt5_DIR}/../../../bin
        ${Qt6_DIR}/../../../bin
        $ENV{QTDIR}/bin
        DOC "Qt Help Generator executable")

find_program(TPAGE_EXECUTABLE NAMES tpage
        DOC "Template Toolkit tpage executable")

# Check if all required tools are available
set(MISSING_TOOLS "")
if(NOT QHELPGENERATOR_EXECUTABLE)
    list(APPEND MISSING_TOOLS "qhelpgenerator")
endif()
if(NOT TPAGE_EXECUTABLE)
    list(APPEND MISSING_TOOLS "tpage (install with: sudo cpan -i Template::Tools)")
endif()

if(MISSING_TOOLS)
    message(STATUS "Documentation build disabled - missing tools: ${MISSING_TOOLS}")
    return()
endif()

message(STATUS "Documentation build enabled")
message(STATUS "  qhelpgenerator: ${QHELPGENERATOR_EXECUTABLE}")
message(STATUS "  tpage: ${TPAGE_EXECUTABLE}")

# Set directories
set(DOC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DOC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DOC_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

# --- Symlink Qt Assistant for macOS development ---
if(APPLE AND QHELPGENERATOR_EXECUTABLE)
    # Start from qhelpgenerator's directory
    get_filename_component(QHELPGEN_DIR "${QHELPGENERATOR_EXECUTABLE}" DIRECTORY)

    # Search common Qt layouts (vcpkg, homebrew, official Qt)
    set(ASSISTANT_CANDIDATES
            "${QHELPGEN_DIR}/Assistant.app"
            "${QHELPGEN_DIR}/../bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt5/bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt5-tools/bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt5/debug/bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt6/bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt6-tools/bin/Assistant.app"
            "${QHELPGEN_DIR}/../../qt6/debug/bin/Assistant.app"
            )

    # Find first existing Assistant.app (resolve symlinks)
    set(ASSISTANT_SOURCE "")
    foreach(candidate IN LISTS ASSISTANT_CANDIDATES)
        get_filename_component(real_path "${candidate}" REALPATH)
        if(EXISTS "${real_path}")
            set(ASSISTANT_SOURCE "${real_path}")
            message(STATUS "Found Qt Assistant: ${ASSISTANT_SOURCE}")
            break()
        endif()
    endforeach()

    if(ASSISTANT_SOURCE)
        # Create symlink target
        add_custom_target(link_qt_assistant ALL
                COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_OUTPUT_DIR}"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${DOC_OUTPUT_DIR}/Assistant.app"
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${DOC_OUTPUT_DIR}/Assistant.app"
                COMMAND ${CMAKE_COMMAND} -E create_symlink
                "${ASSISTANT_SOURCE}"
                "${DOC_OUTPUT_DIR}/Assistant.app"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${DOC_OUTPUT_DIR}/assistant"
                COMMAND ${CMAKE_COMMAND} -E create_symlink
                "Assistant.app/Contents/MacOS/Assistant"
                "${DOC_OUTPUT_DIR}/assistant"
                COMMENT "Symlinking Qt Assistant to ${DOC_OUTPUT_DIR}"
                )
    else()
        message(WARNING
                "Qt Assistant not found in any expected location.\n"
                "Searched: ${ASSISTANT_CANDIDATES}\n"
                "Help system will not work unless Qt Assistant is manually installed to bin/"
                )
    endif()
endif()

# Find all source files for dependencies
file(GLOB_RECURSE BODY_FILES "${DOC_SOURCE_DIR}/*.body")
file(GLOB_RECURSE IMAGE_FILES "${DOC_SOURCE_DIR}/images/*")

# Create the build directory structure and copy all needed files
add_custom_command(
        OUTPUT ${DOC_BUILD_DIR}/.setup_complete
        COMMAND ${CMAKE_COMMAND} -E echo "Setting up documentation build directory..."

        # Copy configuration files
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/header.us3 ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/footer.us3 ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/mainstyle.css ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/manual.qhp ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/manual.qhcp ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/ultrascan3.txt ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/wrapped-images.lst ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/template.img.body ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/make-html.sh ${DOC_BUILD_DIR}/
        COMMAND ${CMAKE_COMMAND} -E copy ${DOC_SOURCE_DIR}/make-wraps.sh ${DOC_BUILD_DIR}/

        # Copy images and body files
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${DOC_SOURCE_DIR}/images ${DOC_BUILD_DIR}/images
        COMMAND bash -c "find '${DOC_SOURCE_DIR}' -type f -name '*.body' ! -path '*/.staging/*' -exec cp {} '${DOC_BUILD_DIR}/' \\;"

        # Make scripts executable
        COMMAND chmod +x ${DOC_BUILD_DIR}/make-html.sh
        COMMAND chmod +x ${DOC_BUILD_DIR}/make-wraps.sh
        COMMAND ${CMAKE_COMMAND} -E touch ${DOC_BUILD_DIR}/.setup_complete

        DEPENDS
        ${BODY_FILES}
        ${IMAGE_FILES}
        ${DOC_SOURCE_DIR}/header.us3
        ${DOC_SOURCE_DIR}/footer.us3
        ${DOC_SOURCE_DIR}/mainstyle.css
        ${DOC_SOURCE_DIR}/manual.qhp
        ${DOC_SOURCE_DIR}/manual.qhcp
        ${DOC_SOURCE_DIR}/ultrascan3.txt
        ${DOC_SOURCE_DIR}/wrapped-images.lst
        ${DOC_SOURCE_DIR}/template.img.body
        ${DOC_SOURCE_DIR}/make-html.sh
        ${DOC_SOURCE_DIR}/make-wraps.sh
        COMMENT "Preparing documentation build environment"
        VERBATIM
)

# Run the image wrapping script
add_custom_command(
        OUTPUT ${DOC_BUILD_DIR}/.wraps_complete
        COMMAND ${CMAKE_COMMAND} -E echo "Wrapping image files..."
        COMMAND bash -c "cd '${DOC_BUILD_DIR}' && ./make-wraps.sh"
        COMMAND ${CMAKE_COMMAND} -E touch ${DOC_BUILD_DIR}/.wraps_complete
        DEPENDS ${DOC_BUILD_DIR}/.setup_complete
        COMMENT "Creating HTML wrappers for images"
        VERBATIM
)

# Process all body files to HTML
add_custom_command(
        OUTPUT ${DOC_BUILD_DIR}/.html_complete
        COMMAND ${CMAKE_COMMAND} -E echo "Processing body files to HTML..."
        COMMAND bash -c "cd '${DOC_BUILD_DIR}' && for file in *.body; do ./make-html.sh \"\${file%.body}\" \"UltraScan Documentation\" -f -q; done"
        COMMAND ${CMAKE_COMMAND} -E touch ${DOC_BUILD_DIR}/.html_complete
        DEPENDS ${DOC_BUILD_DIR}/.wraps_complete
        COMMENT "Converting body files to HTML"
        VERBATIM
)

# Generate Qt help files
add_custom_command(
        OUTPUT ${DOC_OUTPUT_DIR}/manual.qch ${DOC_OUTPUT_DIR}/manual.qhc
        COMMAND ${CMAKE_COMMAND} -E echo "Generating Qt help files..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT_DIR}
        COMMAND bash -c "cd '${DOC_BUILD_DIR}' && ${QHELPGENERATOR_EXECUTABLE} manual.qhp -o '${DOC_OUTPUT_DIR}/manual.qch'"
        COMMAND bash -c "cd '${DOC_BUILD_DIR}' && ${QHELPGENERATOR_EXECUTABLE} manual.qhcp -o '${DOC_OUTPUT_DIR}/manual.qhc'"
        DEPENDS ${DOC_BUILD_DIR}/.html_complete
        COMMENT "Building Qt Assistant documentation files"
        VERBATIM
)

# Create documentation target
add_custom_target(documentation ALL
        DEPENDS ${DOC_OUTPUT_DIR}/manual.qch ${DOC_OUTPUT_DIR}/manual.qhc
        )

# Install documentation files
install(FILES
        ${DOC_OUTPUT_DIR}/manual.qch
        ${DOC_OUTPUT_DIR}/manual.qhc
        DESTINATION bin
        COMPONENT documentation
        )

# For macOS application bundles
if(APPLE)
    install(FILES
            ${DOC_OUTPUT_DIR}/manual.qch
            ${DOC_OUTPUT_DIR}/manual.qhc
            DESTINATION UltraScan.app/Contents/Resources
            COMPONENT documentation
            )
endif()

# Clean target
add_custom_target(clean-documentation
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOC_BUILD_DIR}/.staging
        COMMAND ${CMAKE_COMMAND} -E remove ${DOC_BUILD_DIR}/.setup_complete
        COMMAND ${CMAKE_COMMAND} -E remove ${DOC_BUILD_DIR}/.wraps_complete
        COMMAND ${CMAKE_COMMAND} -E remove ${DOC_BUILD_DIR}/.html_complete
        COMMAND ${CMAKE_COMMAND} -E remove ${DOC_OUTPUT_DIR}/manual.qch
        COMMAND ${CMAKE_COMMAND} -E remove ${DOC_OUTPUT_DIR}/manual.qhc
        COMMENT "Cleaning documentation build files"
        )

# Option to disable documentation build
option(BUILD_DOCUMENTATION "Build Qt Assistant documentation" ON)
if(NOT BUILD_DOCUMENTATION)
    set_target_properties(documentation PROPERTIES EXCLUDE_FROM_ALL TRUE)
    message(STATUS "Documentation build will be skipped (BUILD_DOCUMENTATION=OFF)")
endif()
