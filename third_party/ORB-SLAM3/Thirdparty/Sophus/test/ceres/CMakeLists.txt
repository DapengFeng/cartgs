# Make sure Ceres knows where to find Eigen
list(APPEND SEARCH_HEADERS ${EIGEN3_INCLUDE_DIR})

# git clone https://ceres-solver.googlesource.com/ceres-solver
find_package(Ceres 1.6.0 QUIET)

if(Ceres_FOUND)
    message(STATUS "CERES found")

    # Tests to run
    set(TEST_SOURCES test_ceres_se3)

    foreach(test_src ${TEST_SOURCES})
        add_executable(${test_src} ${test_src}.cpp local_parameterization_se3)
        target_link_libraries(${test_src} sophus ${CERES_LIBRARIES})
        target_include_directories(
            ${test_src}
            SYSTEM
            PRIVATE ${CERES_INCLUDE_DIRS}
        )
        add_test(${test_src} ${test_src})
    endforeach(test_src)
endif(Ceres_FOUND)
