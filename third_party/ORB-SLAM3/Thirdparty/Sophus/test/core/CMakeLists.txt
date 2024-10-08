# Tests to run
set(TEST_SOURCES
    test_common
    test_so2
    test_se2
    test_rxso2
    test_sim2
    test_so3
    test_se3
    test_rxso3
    test_sim3
    test_velocities
    test_geometry
)
find_package(Ceres 1.6.0 QUIET)

foreach(test_src ${TEST_SOURCES})
    add_executable(
        ${test_src}
        ${test_src}.cpp
        tests.hpp
        ../../sophus/test_macros.hpp
    )
    target_link_libraries(${test_src} sophus)
    if(Ceres_FOUND)
        target_include_directories(
            ${test_src}
            SYSTEM
            PRIVATE ${CERES_INCLUDE_DIRS}
        )
        add_definitions(-DSOPHUS_CERES)
    endif(Ceres_FOUND)
    add_test(${test_src} ${test_src})
endforeach(test_src)
