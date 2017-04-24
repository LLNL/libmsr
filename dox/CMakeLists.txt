find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(doc 
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    add_custom_target(latex_doc
        make
        DEPENDS doc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/latex
        COMMENT "Building Doxygen Latex documentation" VERBATIM
    )
endif(DOXYGEN_FOUND)
