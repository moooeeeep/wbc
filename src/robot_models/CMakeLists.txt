add_subdirectory(rbdl)
if(USE_PINOCCHIO)
    add_subdirectory(pinocchio)
endif()
if(USE_HYRODYN)
    add_subdirectory(hyrodyn)
endif()
if(USE_KDL)
    add_subdirectory(kdl)
endif()

