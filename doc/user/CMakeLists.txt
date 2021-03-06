CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

# Turn CMAKE_LIBRARY_PATH list into a string in order to pass all its contents to the CMake command generating the doc,
# only the first list entry is transferred otherwise.
STRING(REPLACE ";" "," CMAKE_LIBRARY_PATH_STR "${CMAKE_LIBRARY_PATH}")

# Ensure that libraries paths are in MSYS format when using MSYS Makefiles,
# otherwise the documentation fails to build
IF("${CMAKE_GENERATOR}" MATCHES ".*MSYS.*")
  TULIP_WINDOWS_TO_MSYS_PATH(${CMAKE_LIBRARY_PATH_STR} CMAKE_LIBRARY_PATH_STR)
ENDIF("${CMAKE_GENERATOR}" MATCHES ".*MSYS.*")

ADD_CUSTOM_TARGET(userDoc ALL ${CMAKE_COMMAND} -DSPHINX_EXECUTABLE=${SPHINX_EXECUTABLE}
                                               -DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
                                               -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                                               -DAPPLE=${APPLE} -DWIN32=${WIN32}
                                               -DCMAKE_LIBRARY_PATH=${CMAKE_LIBRARY_PATH_STR}
                                               -P ${CMAKE_CURRENT_SOURCE_DIR}/genSphinxDoc.cmake
                                               VERBATIM)

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "doc;doctrees") 

INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html 
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/tulip-user
        COMPONENT tulip_user_doc)
