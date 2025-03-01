cmake_minimum_required (VERSION 2.8.11)
project(wbc)

include(FindPkgConfig)

set(PROJECT_VERSION 0.2)
set(API_VERSION ${PROJECT_VERSION})

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
if(USE_HYRODYN AND USE_PINOCCHIO AND USE_KDL AND USE_EIQUADPROG AND USE_QPSWIFT AND USE_PROXQP)
    message("Detected Full install, adding benchmark folder")
    add_subdirectory(benchmarks)
endif()
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tutorials)
add_subdirectory(bin)
if(USE_PYTHON)
    message("USING PYTHON.")
    add_subdirectory(bindings/python)
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    message("Doxygen build started.")
    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_in}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
