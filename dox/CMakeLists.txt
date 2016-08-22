find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(doc 
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    add_custom_target(latex_doc
        cd ${CMAKE_CURRENT_BINARY_DIR}/latex && make
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/latex
        COMMENT "Building Doxygen Latex documentation" VERBATIM
    )
endif(DOXYGEN_FOUND)
